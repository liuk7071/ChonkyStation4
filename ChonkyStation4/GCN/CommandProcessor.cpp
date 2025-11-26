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