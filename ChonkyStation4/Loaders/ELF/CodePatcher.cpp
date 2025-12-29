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

                // Allocate code for the patch.
                // We need the code to be allocated within 2GiB of the original instruction so that we can jump to it with a single relative jmp.
                u8* patch_code_ptr = nullptr;
                auto is_addr_ok = [&](u8* addr) {
                    s64 diff = std::llabs((u8*)addr - instr_addr);
                    return diff <= 0x80000000;
                };
#ifdef _WIN32
                // TODO: Add a timeout or something
                void* addr = code_ptr + offs;
                while (!patch_code_ptr || !is_addr_ok(patch_code_ptr)) {
                    MEMORY_BASIC_INFORMATION mbi;
                    VirtualQuery((void*)addr, &mbi, sizeof(mbi));
                    if (mbi.State == MEM_RESERVE && mbi.RegionSize >= 128) {
                        patch_code_ptr = (u8*)VirtualAlloc(addr, 128, MEM_COMMIT, PAGE_EXECUTE_READWRITE);    // NOTE: WE STUBBED THE ALLOCATION TO 128 BYTES!!!
                    }
                    else patch_code_ptr = nullptr;

                    if (patch_code_ptr && !is_addr_ok(patch_code_ptr)) {
                        VirtualFree(patch_code_ptr, 128, MEM_DECOMMIT);
                        patch_code_ptr = nullptr;
                    }

                    addr = (u8*)((u64)mbi.BaseAddress + mbi.RegionSize);
                }
#else
                Helpers::panic("Unsupported platform\n");
#endif
                auto code = std::make_unique<Xbyak::CodeGenerator>(128, patch_code_ptr);
                //log("Allocated patch at %p\n", patch_code_ptr);

                // This code puts [(gs:[0x58] + _tls_index * 8) + guest_tls_ptr_offs] in the dest register
                // without altering any state other than the dest register.
                code->mov(dest, _tls_index);                            // TLS index for this module
                code->shl(dest, 3);                                     // Multiply by sizeof(u64) (pointer)
                code->putSeg(gs);
                code->add(dest, ptr[0x58]);                             // Get pointer to TLS array (gs:[0x58]), add it to _tls_index * 8
                code->mov(dest, ptr[dest]);                             // Get host TLS pointer
                code->add(dest, PS4::OS::Thread::guest_tls_ptr_offs);   // Add the offset of the guest TLS variable within the host TLS
                code->mov(dest, ptr[dest]);                             // Load guest TLS pointer
                code->jmp(instr_addr + instruction.length);             // Jump back to the next instruction

                // Patch instruction to jmp to our code
                code = std::make_unique<Xbyak::CodeGenerator>(instruction.length, instr_addr);
                code->jmp(patch_code_ptr);
                Helpers::debugAssert(code->getSize() <= instruction.length, "CodePatcher: patch is larger than the original instruction (patch is %d, instruction is %d)\n", code->getSize(), instruction.length);

                const auto leftover = std::max(instruction.length - code->getSize(), (u64)0);
                std::memset(code_ptr + offs + code->getSize(), 0xcd, leftover);
            }

            offs += instruction.length;
        } else offs++;
        
        if (offs >= size) break;
    }

    log("Done\n");
}

}   // End namespace Loader::ELF