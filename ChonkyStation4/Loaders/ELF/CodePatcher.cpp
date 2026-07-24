#include "CodePatcher.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Thread.hpp>
#include <Zydis/Zydis.h>
#include <xbyak/xbyak.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif


extern "C" unsigned long _tls_index;

namespace PS4::Loader::ELF {

MAKE_LOG_FUNCTION(log, loader_elf);

// TODO: Make a more generic patch system

u8* allocatePatchCode(void* addr, size_t size) {
    // Allocate code for the patch.
    // We need the code to be allocated within 2GiB of the original instruction so that we can jump to it with a single relative jmp.
    const u8* instr_addr = (u8*)addr;
    u8* patch_code_ptr = nullptr;

    auto is_addr_ok = [&](u8* to_check) {
        s64 diff = std::llabs((u8*)to_check - instr_addr);
        return diff <= 0x80000000;
    };

#ifdef _WIN32
    // TODO: Add a timeout or something
    while (!patch_code_ptr || !is_addr_ok(patch_code_ptr)) {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery((void*)addr, &mbi, sizeof(mbi));
        if (mbi.State == MEM_RESERVE && mbi.RegionSize >= size) {
            patch_code_ptr = (u8*)VirtualAlloc(addr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        }
        else patch_code_ptr = nullptr;

        if (patch_code_ptr && !is_addr_ok(patch_code_ptr)) {
            VirtualFree(patch_code_ptr, size, MEM_DECOMMIT);
            patch_code_ptr = nullptr;
        }

        addr = (u8*)((u64)mbi.BaseAddress + mbi.RegionSize);
    }
#else
    Helpers::panic("Unsupported platform\n");
#endif

    return patch_code_ptr;
}

void patchCode(Module& module, u8* code_ptr, size_t size) {
    using namespace Xbyak::util;
    
    log("Patching code at %p size %llu\n", code_ptr, size);

    auto zydis_to_xbyak = [](const ZydisRegister r) -> Xbyak::Reg {
        if (r >= ZYDIS_REGISTER_EAX && r <= ZYDIS_REGISTER_R15D)
            return Xbyak::Reg32(r - ZYDIS_REGISTER_EAX + Xbyak::Operand::EAX);
        if (r >= ZYDIS_REGISTER_RAX && r <= ZYDIS_REGISTER_R15)
            return Xbyak::Reg64(r - ZYDIS_REGISTER_RAX + Xbyak::Operand::RAX);
        
        Helpers::panic("zydis_to_xbyak: unhandled register %d\n", r);
    };
   
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    
    size_t offs = 0;
    while (true) {
        if (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, (void*)(code_ptr + offs), size - offs, &instruction, operands))) {
            // Patch "mov dest, fs:[0]" with a jmp to a custom func that directly passes the guest TLS pointer to dst.
            if (    instruction.mnemonic == ZYDIS_MNEMONIC_MOV
                &&  operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY
                &&  operands[1].mem.segment == ZYDIS_REGISTER_FS
                &&  operands[1].mem.base == ZYDIS_REGISTER_NONE
                &&  operands[1].mem.disp.value == 0
                &&  operands[0].reg.value >= ZYDIS_REGISTER_RAX // There seem to be some instructions that move from FS to other segment registers. I don't know if those need to be patched and if so with what
                &&  operands[0].reg.value <= ZYDIS_REGISTER_R15
               ) {
                u8* instr_addr = code_ptr + offs;
                log("- Found TLS access with size %d @ %p\n", instruction.length, instr_addr);

                // Ensure the threading system is initialized
                Helpers::debugAssert(PS4::OS::Thread::initialized, "CodePatcher: threading system is not initialized\n");   // Unreachable in theory

                // Build patch code
                auto dest = zydis_to_xbyak(operands[0].reg.value);

                u8* patch_code_ptr = allocatePatchCode(instr_addr, 128);
                auto code = std::make_unique<Xbyak::CodeGenerator>(128, patch_code_ptr);
                //log("Allocated patch at %p\n", patch_code_ptr);

                // This code puts [[gs:[0x58] + _tls_index * 8] + guest_tls_ptr_offs] in the dest register
                // without altering any state other than the dest register.
                code->putSeg(gs);
                code->mov(dest, ptr[0x58]);
                code->mov(dest, ptr[dest + (_tls_index << 3)]);                     // [gs:[0x58] + _tls_index * 8]
                code->mov(dest, ptr[dest + PS4::OS::Thread::guest_tls_ptr_offs]);
                code->jmp(instr_addr + instruction.length);                         // Jump back to the next instruction

                // Patch instruction to jmp to our code
                code = std::make_unique<Xbyak::CodeGenerator>(instruction.length, instr_addr);
                code->jmp(patch_code_ptr);
                Helpers::debugAssert(code->getSize() <= instruction.length, "CodePatcher: patch is larger than the original instruction (patch is %d, instruction is %d)\n", code->getSize(), instruction.length);

                const auto leftover = std::max((s64)instruction.length - (s64)code->getSize(), (s64)0);
                std::memset(code_ptr + offs + code->getSize(), 0xcd, leftover);
            }

            offs += instruction.length;
        } else offs++;
        
        if (offs >= size) break;
    }

    log("Done\n");
}

// This patch eliminates red zone usage from a function.
// 
// Red zone usage is a problem on Windows when using page faults, because its ABI does not expect a red zone to exist.
// When a page fault happens, the Windows kernel's exception handler pushes structs (that are passed to the userland exception dispatcher, KiUserExceptionDispatcher, and then to the VEH)
// on the same stack as the faulting thread, so if a fault happens in a function that uses the red zone, the kernel ends up corrupting that memory.
// Nothing can be done about this, because by the time userland is reached the damage is done. Patching ntdll's KiUserExceptionDispatcher would not work.
// What we can do is to make our caches protect as little memory as possible, or patch out red zone usage in problematic functions, which is what we do here.
// 
// Although it is theoretically possible, this patch is not meant to be applied automatically to every single function in a game.
// It is supposed to be applied to specific per-game functions that have been confirmed to be problematic and have been analyzed beforehand.
// This is because the function needs to meet a series of conditions.
// 
// The conditions for a function that accesses the stack's red zone to be patched are as follows:
//  - All RSP accesses in the function have a displacement (because we'd need to grow the instructions otherwise).
//  - After patching, the highest displacement needs to be < 128 (otherwise we'd need to grow the instructions).
//  - At the start/end of the function there are enough push/pops to replace them with a jmp to our patch code (5 bytes).
//  - The function does not use the frame pointer (no mov rbp, rsp) (this could be worked around in the future if necessary).
//  - RSP is not used in any instruction that we do not patch here (more instructions can be added as needed).
// 
// The patch works by calculating the lowest displacement that is ever accessed in the function (lowest_disp) and by lowering RSP by this value.
// Then, add lowest_disp to all displacements in the function. This way, every negative offset becomes positive and we "eliminate" the red zone.
// This works, because the addresses of the stack variables remain identical. We simply modify the RSP-relative offsets.
// 
// For example, this:
//      mov rbx, [rsp - 20h]
//      mov rax, [rsp - 10h]
//      mov rcx, [rsp + 10h]
// becomes this:
//      sub rsp, 20h            # because the lowest displacement was -20h
//      mov rbx, [rsp +  0h]
//      mov rax, [rsp + 10h]
//      mov rcx, [rsp + 30h]
//      add rsp, 20h
//
// If a page fault happens between these instructions, the Windows kernel will receive an RSP that is already lowered below the game's red zone,
// making it so that it will not overwrite it.
// The problem is where to insert the sub/add RSP instructions.
// We do it by replacing the start/end sections of the function where it pushes/pops the callee-saved registers on the stack with a jump to our patch code.
// The patch code will contain the RSP addition/subtraction and the pushes/pops we just removed from the original function.
//
// entry_patch_addr/entry_patch_size: address and size of the code region to be copied in our allocated patch code and then replaced with a jump to it.
//      this code is executed *before* RSP is lowered. Generally, this is the portion of code that pushes the callee-saved registers onto the stack.
//      For example, this:
//          entry:
//              push        rbp  
//              push        r15
//              push        r14
//              push        r13
//              push        r12
//              push        rbx
//              
//      becomes this:
//          entry:
//              jmp         patch
//          rest_of_code: 
//              nop
//              nop
//              nop
//              nop
//              nop
//
//          patch:
//              push        rbp  
//              push        r15
//              push        r14
//              push        r13
//              push        r12
//              push        rbx
//              sub         rsp, 30h
//              jmp         rest_of_code
// 
// exit_patch_addr/exit_patch_size: same as the entry, except the code is executed *after* RSP is raised. This is generally the section that pops
//      the previously saved registers.
//
void patchRedZone(u8* func_start, u8* func_end, u8* entry_patch_addr, size_t entry_patch_size, u8* exit_patch_addr, size_t exit_patch_size) {
    using namespace Xbyak::util;

    auto zydis_to_xbyak = [](const ZydisRegister r) -> Xbyak::Reg {
        if (r >= ZYDIS_REGISTER_EAX && r <= ZYDIS_REGISTER_R15D)
            return Xbyak::Reg32(r - ZYDIS_REGISTER_EAX + Xbyak::Operand::EAX);
        if (r >= ZYDIS_REGISTER_RAX && r <= ZYDIS_REGISTER_R15)
            return Xbyak::Reg64(r - ZYDIS_REGISTER_RAX + Xbyak::Operand::RAX);

        Helpers::panic("zydis_to_xbyak: unhandled register %d\n", r);
    };

    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

    struct InstructionToPatch {
        void* address;
        size_t size;
        ZydisDecodedInstruction instruction;
        ZydisDecodedOperand dst;
        ZydisDecodedOperand src;
    };
    std::vector<InstructionToPatch> instructions_to_patch;

    // Copy the entry/exit code
    std::unique_ptr<u8[]> entry_code = std::make_unique<u8[]>(entry_patch_size);
    std::unique_ptr<u8[]> exit_code = std::make_unique<u8[]>(exit_patch_size);
    std::memcpy(entry_code.get(), entry_patch_addr, entry_patch_size);
    std::memcpy(exit_code.get(), exit_patch_addr, exit_patch_size);
    std::memset(entry_patch_addr, 0x90 /* nop */, entry_patch_size);
    std::memset(exit_patch_addr, 0x90 /* nop */, exit_patch_size);

    const auto size = func_end - func_start;
    s32 lowest_disp = 1;
    size_t offs = 0;
    while (true) {
        if (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, (void*)(func_start + offs), size - offs, &instruction, operands))) {
            const auto& dst = operands[0];
            const auto& src = operands[1];
            u8* instr_addr = func_start + offs;

            bool found_instr_to_patch = false;

            // mov reg, [rsp + disp]
            if (   instruction.mnemonic == ZYDIS_MNEMONIC_MOV
                && dst.type             == ZYDIS_OPERAND_TYPE_REGISTER
                && src.type             == ZYDIS_OPERAND_TYPE_MEMORY
                && src.mem.base         == ZYDIS_REGISTER_RSP
               ) {
                if (src.mem.index != ZYDIS_REGISTER_NONE)
                    Helpers::panic("ELF::patchRedZone: red zone access with index register\n");

                const auto disp = src.mem.disp.value;
                log("- Found red zone read with size %d @ %p (disp: %d)\n", instruction.length, instr_addr, disp);

                found_instr_to_patch = true;
                if (disp < lowest_disp)
                    lowest_disp = disp;
            }

            // mov [rsp + disp], reg
            if (   instruction.mnemonic == ZYDIS_MNEMONIC_MOV
                && src.type             == ZYDIS_OPERAND_TYPE_REGISTER
                && dst.type             == ZYDIS_OPERAND_TYPE_MEMORY
                && dst.mem.base         == ZYDIS_REGISTER_RSP
                ) {
                if (src.mem.index != ZYDIS_REGISTER_NONE)
                    Helpers::panic("ELF::patchRedZone: red zone access with index register\n");

                const auto disp = dst.mem.disp.value;
                log("- Found red zone write with size %d @ %p (disp: %d)\n", instruction.length, instr_addr, disp);

                found_instr_to_patch = true;
                if (disp < lowest_disp)
                    lowest_disp = disp;
            }

            // add reg, [rsp + disp]
            if (   instruction.mnemonic == ZYDIS_MNEMONIC_ADD
                && dst.type             == ZYDIS_OPERAND_TYPE_REGISTER
                && src.type             == ZYDIS_OPERAND_TYPE_MEMORY
                && src.mem.base         == ZYDIS_REGISTER_RSP
                ) {
                if (src.mem.index != ZYDIS_REGISTER_NONE)
                    Helpers::panic("ELF::patchRedZone: red zone access with index register\n");

                const auto disp = src.mem.disp.value;
                log("- Found red zone add with size %d @ %p (disp: %d)\n", instruction.length, instr_addr, disp);

                found_instr_to_patch = true;
                if (disp < lowest_disp)
                    lowest_disp = disp;
            }

            // sub reg, [rsp + disp]
            if (   instruction.mnemonic == ZYDIS_MNEMONIC_SUB
                && dst.type             == ZYDIS_OPERAND_TYPE_REGISTER
                && src.type             == ZYDIS_OPERAND_TYPE_MEMORY
                && src.mem.base         == ZYDIS_REGISTER_RSP
                ) {
                if (src.mem.index != ZYDIS_REGISTER_NONE)
                    Helpers::panic("ELF::patchRedZone: red zone access with index register\n");

                const auto disp = src.mem.disp.value;
                log("- Found red zone sub with size %d @ %p (disp: %d)\n", instruction.length, instr_addr, disp);

                found_instr_to_patch = true;
                if (disp < lowest_disp)
                    lowest_disp = disp;
            }

            // cmp [rsp + disp], imm
            if (   instruction.mnemonic == ZYDIS_MNEMONIC_CMP
                && src.type             == ZYDIS_OPERAND_TYPE_IMMEDIATE
                && dst.type             == ZYDIS_OPERAND_TYPE_MEMORY
                && dst.mem.base         == ZYDIS_REGISTER_RSP
                ) {
                if (src.mem.index != ZYDIS_REGISTER_NONE)
                    Helpers::panic("ELF::patchRedZone: red zone access with index register\n");

                const auto disp = dst.mem.disp.value;
                log("- Found red zone cmp with size %d @ %p (disp: %d)\n", instruction.length, instr_addr, disp);

                found_instr_to_patch = true;
                if (disp < lowest_disp)
                    lowest_disp = disp;
            }

            if (found_instr_to_patch) {
                instructions_to_patch.push_back({ .address = instr_addr, .size = instruction.length, .instruction = instruction, .dst = dst, .src = src });
            }

            offs += instruction.length;
        }
        else offs++;

        if (offs >= size) break;
    }

    log("- The lowest displacement is %d\n", lowest_disp);

    // Add lowest_disp to the instructions
    for (auto& to_patch : instructions_to_patch) {
        auto code = std::make_unique<Xbyak::CodeGenerator>(to_patch.size, to_patch.address);
        std::memset(to_patch.address, 0x90 /* nop */, to_patch.size);
        
        // mov reg, [rsp + disp]
        if (   to_patch.instruction.mnemonic    == ZYDIS_MNEMONIC_MOV
            && to_patch.dst.type                == ZYDIS_OPERAND_TYPE_REGISTER
            && to_patch.src.type                == ZYDIS_OPERAND_TYPE_MEMORY
            && to_patch.src.mem.base            == ZYDIS_REGISTER_RSP
           ) {
            code->mov(zydis_to_xbyak(to_patch.dst.reg.value), ptr[rsp + (to_patch.src.mem.disp.value + -lowest_disp)]);
        }

        // mov [rsp + disp], reg
        if (   to_patch.instruction.mnemonic    == ZYDIS_MNEMONIC_MOV
            && to_patch.src.type                == ZYDIS_OPERAND_TYPE_REGISTER
            && to_patch.dst.type                == ZYDIS_OPERAND_TYPE_MEMORY
            && to_patch.dst.mem.base            == ZYDIS_REGISTER_RSP
            ) {
            code->mov(ptr[rsp + (to_patch.dst.mem.disp.value + -lowest_disp)], zydis_to_xbyak(to_patch.src.reg.value));
        }

        // add reg, [rsp + disp]
        if (   to_patch.instruction.mnemonic    == ZYDIS_MNEMONIC_ADD
            && to_patch.dst.type                == ZYDIS_OPERAND_TYPE_REGISTER
            && to_patch.src.type                == ZYDIS_OPERAND_TYPE_MEMORY
            && to_patch.src.mem.base            == ZYDIS_REGISTER_RSP
           ) {
            code->add(zydis_to_xbyak(to_patch.dst.reg.value), ptr[rsp + (to_patch.src.mem.disp.value + -lowest_disp)]);
        }

        // sub reg, [rsp + disp]
        if (   to_patch.instruction.mnemonic    == ZYDIS_MNEMONIC_SUB
            && to_patch.dst.type                == ZYDIS_OPERAND_TYPE_REGISTER
            && to_patch.src.type                == ZYDIS_OPERAND_TYPE_MEMORY
            && to_patch.src.mem.base            == ZYDIS_REGISTER_RSP
           ) {
            code->sub(zydis_to_xbyak(to_patch.dst.reg.value), ptr[rsp + (to_patch.src.mem.disp.value + -lowest_disp)]);
        }

        // cmp [rsp + disp], imm
        if (   to_patch.instruction.mnemonic    == ZYDIS_MNEMONIC_CMP
            && to_patch.src.type                == ZYDIS_OPERAND_TYPE_IMMEDIATE
            && to_patch.dst.type                == ZYDIS_OPERAND_TYPE_MEMORY
            && to_patch.dst.mem.base            == ZYDIS_REGISTER_RSP
            ) {
            switch (to_patch.dst.size) {
            
            case 8:  code->cmp(Xbyak::util::byte [rsp + (to_patch.dst.mem.disp.value + -lowest_disp)], to_patch.src.imm.value.u); break;
            case 16: code->cmp(Xbyak::util::word [rsp + (to_patch.dst.mem.disp.value + -lowest_disp)], to_patch.src.imm.value.u); break;
            case 32: code->cmp(Xbyak::util::dword[rsp + (to_patch.dst.mem.disp.value + -lowest_disp)], to_patch.src.imm.value.u); break;
            case 64: code->cmp(Xbyak::util::qword[rsp + (to_patch.dst.mem.disp.value + -lowest_disp)], to_patch.src.imm.value.u); break;

            default:
                Helpers::panic("Unsupported cmp operand size %d", to_patch.dst.size);
            }
        }
        
        Helpers::debugAssert(code->getSize() <= to_patch.size, "CodePatcher: patch is larger than the original instruction (patch is %d, instruction is %d)\n", code->getSize(), to_patch.size);
    }

    // Build prologue patch
    u8* patch_code_ptr = allocatePatchCode(entry_patch_addr, 1_KB);
    auto code = std::make_unique<Xbyak::CodeGenerator>(1_KB, patch_code_ptr);
    
    // Leave nops to copy the function's entry code later.
    void* copy_dest = (void*)code->getCurr();
    for (size_t i = 0; i < entry_patch_size; i++)
        code->nop();

    // Copy the function's entry we replaced.
    std::memcpy(copy_dest, entry_code.get(), entry_patch_size);
    
    // Lower RSP by lowest_disp.
    code->sub(rsp, -lowest_disp);

    // Jump back.
    code->jmp(entry_patch_addr + entry_patch_size);

    // Patch the function's entry to jump to our code.
    code = std::make_unique<Xbyak::CodeGenerator>(entry_patch_size, entry_patch_addr);
    code->jmp(patch_code_ptr);

    // Build epilogue patch
    patch_code_ptr = allocatePatchCode(entry_patch_addr, 1_KB);
    code = std::make_unique<Xbyak::CodeGenerator>(1_KB, patch_code_ptr);

    // First, raise RSP by lowest_disp.
    code->add(rsp, -lowest_disp);

    // Leave nops to copy the function's exit code later.
    copy_dest = (void*)code->getCurr();
    for (size_t i = 0; i < exit_patch_size; i++)
        code->nop();

    // Copy the function's exit we replaced.
    std::memcpy(copy_dest, exit_code.get(), exit_patch_size);

    // Return
    code->ret();

    // Patch the function's exit to jump to our code.
    code = std::make_unique<Xbyak::CodeGenerator>(exit_patch_size, exit_patch_addr);
    code->jmp(patch_code_ptr);

    log("Done\n");
}

}   // End namespace Loader::ELF