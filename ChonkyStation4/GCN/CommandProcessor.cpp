#include "CommandProcessor.hpp"
#include <Logger.hpp>
#include <GCN/PM4.hpp>
#include <BitField.hpp>
#include <thread>
#include <atomic>


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
    BitField<0, 2, u32> swap;
    BitField<2, 30, u32> poll_addr_lo;
};

union DmaData1 {
    u32 raw;
    BitField<0, 1, u32> engine;
    BitField<12, 1, u32> src_atc;
    BitField<13, 2, u32> src_cache_policy;
    BitField<15, 1, u32> src_volatile;
    BitField<20, 2, DmaData::DmaDataDst> dst_sel;
    BitField<24, 1, u32> dst_atc;
    BitField<25, 2, u32> dst_cache_policy;
    BitField<27, 1, u32> dst_volatile;
    BitField<29, 2, DmaData::DmaDataSrc> src_sel;
    BitField<31, 1, u32> cp_sync;
};

union MemSemaphore1 {
    u32 raw;
    BitField<3, 29, u32> addr_lo;
};

union MemSemaphore2 {
    u32 raw;
    BitField<0,  8, u32> addr_hi;
    BitField<16, 1, u32> use_mailbox;
    BitField<20, 1, MemSemaphore::SignalType> signal_type;
    BitField<24, 2, MemSemaphore::ClientCode> client_code;
    BitField<29, 3, MemSemaphore::Select> sem_sel;
};

union IndirectBuffer1 {
    u32 raw;
    BitField<0, 16, u32> addr_hi;
};

union IndirectBuffer2 {
    u32 raw;
    BitField<0, 20, u32> size;
    BitField<20, 1, u32> chain;
    BitField<24, 8, u32> vmid;
};

union WriteData1 {
    u32 raw;
    BitField<8,  4, u32> dst_sel;
    BitField<16, 1, u32> wr_one_addr;
    BitField<20, 1, u32> wr_confirm;
    BitField<30, 2, u32> engine_sel;
};

// Constant engine
std::atomic<u32> ce_count = 0;
std::atomic<u32> de_count = 0;
std::thread ce_thread;
u8 constant_ram[48_KB];

void initCommandProcessor() {
    std::memset(constant_ram, 0, 48_KB);
}

void processCcb(u32* ccb, size_t ccb_size) {
    if (ce_thread.joinable()) ce_thread.join();

    ce_thread = std::thread([=]() {
        for (u32* ptr = ccb; (u8*)ptr < (u8*)ccb + ccb_size; ) {
            PM4Header* pkt = (PM4Header*)ptr;
            u32* args = ptr;
            args++;

            if (pkt->type == 0) {
                ptr++;
                continue;
                Helpers::panic("CCB PM4 type 0 packet\n");
            }
            else if (pkt->type == 1) {
                Helpers::panic("CCB PM4 type 1 packet\n");
            }
            else if (pkt->type == 2) {
                Helpers::panic("CCB PM4 type 1 packet\n");
            }

            switch ((PM4ItOpcode)(u32)pkt->opcode) {
            case PM4ItOpcode::Nop:  break;

            case PM4ItOpcode::WriteConstRam: {
                const u32 offs = *args++;
                std::memcpy(&constant_ram[offs], args, pkt->count * sizeof(u32));
                break;
            }

            case PM4ItOpcode::DumpConstRam: {
                const u32 offs = *args++;
                const u32 n_dw = *args++;
                const u32 addr_lo = *args++;
                const u32 addr_hi = *args++;
                void* addr = (void*)(addr_lo | ((u64)addr_hi << 32));
                std::memcpy(addr, &constant_ram[offs], n_dw * sizeof(u32));
                break;
            }

            case PM4ItOpcode::IncrementCeCounter: {
                ce_count++;
                break;
            }

            case PM4ItOpcode::WaitOnDeCounterDiff: {
                const u32 diff = *args++;
                while (de_count - ce_count >= diff) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1000));
                    std::this_thread::yield();
                }
                break;
            }

            default: {
                //log("Unimplemented ccb opcode 0x%x count %d\n", (u32)pkt->opcode, (u32)pkt->count);
                break;
            }
            }

            ptr += pkt->count + 2;
        }
    });
}

void* index_base = nullptr;
s32   n_indices = 0;
void* indirect_args_base = nullptr;

void processCommands(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size) {
    if (ccb) {
        processCcb(ccb, ccb_size);
    }

    for (u32* ptr = dcb; (u8*)ptr < (u8*)dcb + dcb_size; ) {
        PM4Header* pkt = (PM4Header*)ptr;
        u32* args = ptr;
        args++;
        
        if (pkt->type == 0) {
            ptr++;
            continue;
            Helpers::panic("PM4 type 0 packet\n");
        }
        else if (pkt->type == 1) {
            Helpers::panic("PM4 type 1 packet\n");
        }
        else if (pkt->type == 2) {
            printf("Encountered type 2 packet\n");
            ptr++;
            continue;
        }

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

        case PM4ItOpcode::SetBase: {
            const u32 base_idx = *args++;
            const u32 addr_lo = *args++;
            const u32 addr_hi = *args++;

            switch (base_idx) {
            
            // DrawIndexIndirect
            case 1: {
                indirect_args_base = (void*)(addr_lo | ((u64)(addr_hi & 0xffff) << 32));
                break;
            }

            default:    log("Encountered SetBase packet with unhandled base_idx %d\n", base_idx);
            }
            break;
        }

        case PM4ItOpcode::IndexBufferSize: {
            n_indices = *args++;
            break;
        }

        case PM4ItOpcode::WriteData: {
            const WriteData1 d1 = { .raw = *args++ };
            const u64 addr = *args | ((u64)(*(args + 1)) << 32);
            args += 2;
            
            if (d1.wr_one_addr) {
                // TODO: Does this mean we don't increment the destination or the source address
                Helpers::panic("WriteData: wr_one_addr\n");
            }

            std::memcpy((void*)addr, args, (pkt->count - 2) * sizeof(u32));
            break;
        }

        case PM4ItOpcode::MemSemaphore: {
            using namespace MemSemaphore;
            const MemSemaphore1 d1 = { .raw = *args++ };
            const MemSemaphore2 d2 = { .raw = *args++ };
            u64* sem_ptr = (u64*)(((u64)d1.addr_lo << 3) | ((u64)d2.addr_hi << 32));
            
            log("MemSemaphore: ptr=%p\n", sem_ptr);
            switch (d2.sem_sel) {
            case Select::SignalSemaphore: {
                log("Signalling semaphore\n");
                switch (d2.signal_type) {
                case SignalType::Increment: (*ptr)++;   break;
                case SignalType::Write:     *ptr = 1;   break;

                default: Helpers::panic("invalid MemSemaphore signal type %d\n", d2.signal_type.Value());
                }
                break;
            }

            case Select::WaitSemaphore: {
                log("Waiting on semaphore\n");
                while (*ptr == 0) std::this_thread::sleep_for(std::chrono::microseconds(10));
                break;
            }

            default: Helpers::panic("invalid MemSemaphore mode %d\n", d2.sem_sel.Value());
            }
            break;
        }

        case PM4ItOpcode::WaitRegMem: {
            using namespace WaitRegMem;
            const WaitRegMem1 d1 = { .raw = *args++ };
            const WaitRegMem2 d2 = { .raw = *args++ };
            const u32 poll_addr_hi = *args++;
            const u32 reference = *args++;
            const u32 mask = *args++;
            const u32 poll_interval = *args++;
            u32* ptr = (u32*)((d2.poll_addr_lo << 2) | ((u64)poll_addr_hi << 32));
            log("lo 0x%llx hi 0x%llx\n", d2.poll_addr_lo.Value(), poll_addr_hi);
            log("WaitRegMem: mem_space=%d\n", d1.mem_space.Value());
            log("WaitRegMem: ptr=%p\n", ptr);
            log("WaitRegMem: func=%d\n", d1.function.Value());
            log("WaitRegMem: ref=%d\n", reference);

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

            //while (!check()) {
                // TODO: Use poll_interval
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            //}
            break;
        }

        case PM4ItOpcode::IndirectBuffer: {
            const u32 addr_lo = *args++;
            const IndirectBuffer1 d1 = { .raw = *args++ };
            const IndirectBuffer2 d2 = { .raw = *args++ };
            const u32* ptr = (u32*)(addr_lo | ((u64)d1.addr_hi << 32));
            log("IndirectBuffer: ptr=%p, size=0x%llx\n", ptr, d2.size.Value());
            processCommands((u32*)ptr, d2.size, nullptr, 0);
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
            case 4: {
                // TODO: gpu perf counter
                const u64 dummy = 100000;
                std::memcpy(ptr, &dummy, sizeof(u64));
                break;
            }
            default: Helpers::panic("EventWriteEop: unhandled data_sel %d\n", data_sel);
            }

            if (int_sel != 0)   // None
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

        case PM4ItOpcode::DmaData: {
            const DmaData1 info = { .raw = *args++ };
            const u32 src_addr_lo = *args++;
            const u32 src_addr_hi = *args++;
            const u32 dst_addr_lo = *args++;
            const u32 dst_addr_hi = *args++;
            const u32 size = *args++ & 0x1fffff;

            if (dst_addr_lo == 0x3022c) break;

            bool is_fill = false;
            const auto& fill_data = src_addr_lo;

            void* dst;
            void* src;
            switch (info.dst_sel) {
            case DmaData::DmaDataDst::Memory:
            case DmaData::DmaDataDst::MemoryUsingL2:    dst = (void*)(dst_addr_lo | ((u64)dst_addr_hi << 32));  break;
            case DmaData::DmaDataDst::Gds:              log("TODO: GDS TRANSFER\n"); dst = 0;                   break;
            default:
                Helpers::panic("DmaData: unhandled dst_sel %d\n", info.dst_sel.Value());
            }

            switch (info.src_sel) {
            case DmaData::DmaDataSrc::Memory:
            case DmaData::DmaDataSrc::MemoryUsingL2:    src = (void*)(src_addr_lo | ((u64)src_addr_hi << 32));  break;
            case DmaData::DmaDataSrc::Data:             is_fill = true;                                         break;
            default:
                Helpers::panic("DmaData: unhandled src_sel %d\n", info.src_sel.Value());
            }

            log("DmaData: dst=%p, src=%p, size=%d\n", dst, src, size);

            if (is_fill) {
                if (dst)
                    std::memset(dst, fill_data, size);
            }
            else {
                if (dst && src)
                    std::memcpy(dst, src, size);
            }
            break;
        }

        case PM4ItOpcode::SetConfigReg: {
            const u32 reg_offset = 0x2000 + *args++;    // 0x2000 is the offset for ConfigReg
            log("Set context register 0x%x\n", reg_offset);
            if (reg_offset < 0xd000)
                std::memcpy(&renderer->regs[reg_offset], args, pkt->count * sizeof(u32));
            else printf("Bad config register offset 0x%x\n", reg_offset);
            break;
        }

        case PM4ItOpcode::SetContextReg: {
            const u32 reg_offset = 0xa000 + *args++;    // 0xa000 is the offset for ContextReg
            log("Set context register 0x%x\n", reg_offset);
            if (reg_offset < 0xd000)
                std::memcpy(&renderer->regs[reg_offset], args, pkt->count * sizeof(u32));
            else printf("Bad context register offset 0x%x\n", reg_offset);

            // This hack is ported from shadPS4.
            // We can't rely on the hardware pitch/slice registers to figure out the size of the render targets, because
            // they are always forcefully aligned to tile size (8x8). This leads to inaccurate sizes (i.e. 1920x1088 for 1080p).
            // Thankfully for us games seem to always insert a hint packet that contains the actual surface dimensions, which is what we catch here.
            switch (reg_offset) {
            case Reg::mmCB_COLOR0_BASE:
            case Reg::mmCB_COLOR1_BASE:
            case Reg::mmCB_COLOR2_BASE:
            case Reg::mmCB_COLOR3_BASE:
            case Reg::mmCB_COLOR4_BASE:
            case Reg::mmCB_COLOR5_BASE:
            case Reg::mmCB_COLOR6_BASE:
            case Reg::mmCB_COLOR7_BASE: {
                const auto rt_id = (reg_offset - Reg::mmCB_COLOR0_BASE) / (Reg::mmCB_COLOR1_BASE - Reg::mmCB_COLOR0_BASE);
                Helpers::debugAssert(rt_id < 8, "SetContextReg: invalid rt_id\n");
                
                const auto nop_offs = pkt->count;
                if (nop_offs == 0xe || nop_offs == 0xd || nop_offs == 0xb) {
                    Helpers::debugAssert(args[nop_offs] == 0xc0001000, "CB hint is missing\n");
                    renderer->color_rt_dim[rt_id].raw = args[nop_offs + 1];
                }
                else {
                    renderer->color_rt_dim[rt_id].raw = 0;
                }
                break;
            }

            case Reg::mmCB_COLOR0_CMASK:
            case Reg::mmCB_COLOR1_CMASK:
            case Reg::mmCB_COLOR2_CMASK:
            case Reg::mmCB_COLOR3_CMASK:
            case Reg::mmCB_COLOR4_CMASK:
            case Reg::mmCB_COLOR5_CMASK:
            case Reg::mmCB_COLOR6_CMASK:
            case Reg::mmCB_COLOR7_CMASK: {
                const auto rt_id = (reg_offset - Reg::mmCB_COLOR0_CMASK) / (Reg::mmCB_COLOR1_CMASK - Reg::mmCB_COLOR0_CMASK);
                Helpers::debugAssert(rt_id < 8, "SetContextReg: invalid rt_id\n");

                const auto nop_offs = pkt->count;
                if (nop_offs == 0x4) {
                    Helpers::debugAssert(args[nop_offs] == 0xc0001000, "CB hint is missing\n");
                    renderer->color_rt_dim[rt_id].raw = args[nop_offs + 1];
                }
                break;
            }

            case Reg::mmDB_Z_INFO: {
                if (pkt->count == 8) {
                    Helpers::debugAssert(args[20] == 0xc0001000, "DB hint is missing\n");
                    renderer->depth_rt_dim.raw = args[21];
                }
                else {
                    renderer->depth_rt_dim.raw = 0;
                }
                break;
            }
            }
            break;
        }

        case PM4ItOpcode::SetShReg: {
            const u32 reg_offset = 0x2c00 + *args++;    // 0x2c00 is the offset for ShReg
            log("Set shader register 0x%x\n", reg_offset);
            if (reg_offset < 0xd000)
                std::memcpy(&renderer->regs[reg_offset], args, pkt->count * sizeof(u32));
            else printf("Bad shader register offset 0x%x\n", reg_offset);
            break;
        }

        case PM4ItOpcode::SetUconfigReg: {
            const u32 reg_offset = 0xc000 + *args++;    // 0xc000 is the offset for UconfigReg
            log("Set Uconfig register 0x%x\n", reg_offset);
            if (reg_offset < 0xd000)
                std::memcpy(&renderer->regs[reg_offset], args, pkt->count * sizeof(u32));
            else printf("Bad shader register offset 0x%x\n", reg_offset);
            break;
        }

        case PM4ItOpcode::DrawIndexAuto: {
            const u32 cnt = *args++;
            //const u32 draw_initiator = *args++;
            renderer->draw(cnt);
            break;
        }
        
        case PM4ItOpcode::DrawIndexIndirect: {
            const u32 draw_args_offs = *args++;
            // TODO: BASE_VTX_LOC
            renderer->drawIndirect(1, true, (void*)((uptr)indirect_args_base + draw_args_offs), index_base, n_indices);
            break;
        }

        case PM4ItOpcode::IndexBase: {
            const u32 index_base_lo = *args++;
            const u32 index_base_hi = *args++;
            index_base = (void*)(index_base_lo | ((u64)index_base_hi << 32));
            break;
        }

        case PM4ItOpcode::DrawIndex2: {
            const u32 max_cnt = *args++;
            const u32 index_base_lo = *args++;
            const u32 index_base_hi = *args++;
            const u32 cnt = *args++;
            //const u32 draw_initiator = *args++;
            const void* index_buf_ptr = (void*)(index_base_lo | ((u64)index_base_hi << 32));
            renderer->draw(cnt, index_buf_ptr);
            break;
        }

        case PM4ItOpcode::IndexType: {
            renderer->index_type = (*args & 1) == 0 ? IndexType::Uint16 : IndexType::Uint32;
            break;
        }

        case PM4ItOpcode::IncrementDeCounter: {
            de_count++;
            break;
        }

        case PM4ItOpcode::WaitOnCeCounter: {
            while (ce_count <= de_count) {
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
                std::this_thread::yield();
            }
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