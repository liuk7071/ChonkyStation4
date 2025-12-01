#include "CommandProcessor.hpp"
#include <Logger.hpp>
#include <GCN/PM4.hpp>
#include <BitField.hpp>
#include <thread>


namespace PS4::GCN {

MAKE_LOG_FUNCTION(log, gcn_command_processor);

union WaitRegMem1 {
    u32 raw;
    BitField<0, 3, WaitRegMem::Function> function;
    BitField<4, 1, WaitRegMem::MemSpace> mem_space;
    BitField<8, 1, WaitRegMem::Engine> engine;
};

union WaitRegMem2 {
    u32 raw;
    BitField<0, 16, u32> reg;
    BitField<2, 30, u32> poll_addr_lo;
    BitField<0, 2, u32> swap;
};

void processCommands(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size) {
    // TODO: What is ccb???????? (compute?)
    Helpers::debugAssert(!ccb, "processCommands: ccb != nullptr\n");

    for (u32* ptr = dcb; ptr < dcb + dcb_size; ) {
        if (*ptr == 0) {
            ptr++;
            continue;
        }

        PM4Header* pkt = (PM4Header*)ptr;
        u32* args = ptr;
        args++;
        
        switch ((PM4ItOpcode)(u32)pkt->opcode) {
        case PM4ItOpcode::Nop: {
            u32 cmd = *args++;
            log("nop cmd: 0x%08x\n", cmd);

            switch (cmd) {
            case 0x68750778: {  // Flip with label (?)
                const u32 addr_lo = *args++ & ~3;
                const u32 addr_hi = *args++;
                const u32 data = *args++;
                u32* ptr = (u32*)(addr_lo | ((u64)addr_hi << 32));
                *ptr = data;
                break;
            }
            }

            break;
        }

        case PM4ItOpcode::WaitRegMem: {
            using namespace WaitRegMem;
            WaitRegMem1 d1 = { .raw = *args++ };
            WaitRegMem2 d2 = { .raw = *args++ };
            const u32 poll_addr_hi = *args++;
            const u32 reference = *args++;
            const u32 mask = *args++;
            const u32 poll_interval = *args++;
            u32* ptr = (u32*)((d2.poll_addr_lo << 2) | ((u64)poll_addr_hi << 32));
            log("WaitRegMem: mem_space=%d\n", d1.mem_space.Value());
            log("WaitRegMem: ptr=%p\n", ptr);

            auto check = [&]() -> bool {
                u32 val = (d1.mem_space == MemSpace::Memory) ? *ptr : renderer->regs[d2.reg];
                val &= mask;

                switch (d1.function) {
                case Function::Always:              return true;
                case Function::LessThan:            return val <  reference;
                case Function::LessThanEqual:       return val <= reference;
                case Function::Equal:               return val == reference;
                case Function::NotEqual:            return val != reference;
                case Function::GreaterThanEqual:    return val >= reference;
                case Function::GreaterThan:         return val >  reference;
                default: Helpers::panic("WaitRegMem: unhandled function %d\n", d1.function.Value());
                }
            };

            while (!check()) {
                // TODO: Use poll_interval
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }
            break;
        }

        case PM4ItOpcode::EventWriteEop: {
            const u32 event_ctrl = *args++;
            const u32 addr_lo = *args++;
            const u32 data_ctrl = *args++;
            const u32 data_lo = *args++;
            const u32 data_hi = *args++;

            void* ptr = (void*)(addr_lo | ((u64)(data_ctrl & 0xffff) << 32));
            const u64 data = data_lo | ((u64)data_hi << 32);
            const u32 int_sel = (data_ctrl >> 24) & 3;
            const u32 data_sel = (data_ctrl >> 29) & 7;
            switch (data_sel) {
            case 0:                                             break;      // None
            case 1: std::memcpy(ptr, &data_lo, sizeof(u32));    break;      // 32bit
            case 2: std::memcpy(ptr, &data,    sizeof(u64));    break;      // 64bit
            default: Helpers::panic("EventWriteEop: unhandled data_sel %d\n", data_sel);
            }

            // TODO: I don't think I'm supposed to always trigger the event
            GCN::eop_ev_source.trigger(EOP_EVENT_ID);
            break;
        }

        case PM4ItOpcode::EventWriteEos: {
            const u32 event_ctrl = *args++;
            const u32 addr_lo = *args++;
            const u32 cmd_ctrl = *args++;
            const u32 data = *args++;

            void* ptr = (void*)(addr_lo | ((u64)(cmd_ctrl & 0xffff) << 32));
            const u32 cmd = (cmd_ctrl >> 29) & 7;
            switch (cmd) {
            case 2: std::memcpy(ptr, &data, sizeof(u32));    break;
            default: Helpers::panic("EventWriteEos: unhandled cmd %d\n", cmd);
            }
            break;
        }

        case PM4ItOpcode::SetShReg: {
            const u32 reg_offset = 0x2c00 + *args++;    // 2c00 is the offset for ShReg
            log("Set register 0x%x\n", reg_offset);
            std::memcpy(&renderer->regs[reg_offset], args, (pkt->count + 2 - 1) * sizeof(u32));
            break;
        }

        case PM4ItOpcode::DrawIndexAuto: {
            const u32 cnt = *args++;
            renderer->draw(cnt);
            break;
        }
        
        case PM4ItOpcode::DrawIndex2: {
            const u32 cnt = *args++;
            renderer->draw(cnt);
            break;
        }

        default: {
            log("Unimplemented opcode 0x%x count %d\n", (u32)pkt->opcode, (u32)pkt->count);
            break;
        }
        }
        
        ptr += pkt->count + 2;
    }
}

}   // End namespace PS4::GCN