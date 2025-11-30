#include "CommandProcessor.hpp"
#include <Logger.hpp>
#include <GCN/PM4.hpp>


namespace PS4::GCN {

MAKE_LOG_FUNCTION(log, gcn_command_processor);

void processCommands(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size) {
    // TODO: What is ccb???????? (compute?)
    Helpers::debugAssert(!ccb, "processCommands: ccb != nullptr\n");

    for (u32* ptr = dcb; ptr < dcb + dcb_size; ) {
        PM4Header* pkt = (PM4Header*)ptr;
        u32* args = ptr;
        args++;
        
        switch ((PM4ItOpcode)(u32)pkt->opcode) {
        case PM4ItOpcode::WaitRegMem: {
            union {
                BitField<0, 3, WaitRegMem::Function> function;
                BitField<4, 1, WaitRegMem::MemSpace> mem_space;
                BitField<8, 1, WaitRegMem::Engine> engine;
                u32 raw;
            };
            break;
        }

        case PM4ItOpcode::EventWriteEop: {
            const u32 event_ctrl = *args++;
            const u32 addr_lo = *args++;
            const u32 data_ctrl = *args++;
            const u32 data_lo = *args++;
            const u32 data_hi = *args++;

            void* ptr = (void*)(addr_lo | ((u64)(data_ctrl & 0xffff) << 32));
            const u32 data = data_lo | ((u64)data_hi << 32);
            const u32 int_sel = (data_ctrl >> 24) & 3;
            const u32 data_sel = (data_ctrl >> 29) & 7;
            switch (data_sel) {
            case 0:                                             break;      // None
            case 1: std::memcpy(ptr, &data_lo, sizeof(u32));    break;      // 32bit
            case 2: std::memcpy(ptr, &data,    sizeof(u64));    break;      // 64bit
            default: Helpers::panic("EventWriteEop: unhandled data_sel %d\n", data_sel);
            }

            // TODO: EOP event
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
            //renderer->draw(cnt);
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