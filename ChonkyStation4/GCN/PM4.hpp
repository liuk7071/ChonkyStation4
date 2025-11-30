#pragma once

#include <Common.hpp>
#include <BitField.hpp>


#define PM4_HEADER_BUILD(op, len) (((u32)((((u16)(len) - 2) << 16) | 0xC0000000)) | ((u8)(op)) << 8)

namespace PS4::GCN {

union PM4Header {
    u32 raw;
    // Type 3 packet. Unsure if I need the others
    BitField<0, 1, u32> predicate;
    BitField<1, 1, u32> shader_type;
    BitField<8, 8, u32> opcode;
    BitField<16, 14, u32> count;
    BitField<30, 2, u32> type;
};

namespace WaitRegMem {

enum class Engine : u32 {
    Me = 0u,
    Pfp = 1u
};
enum class MemSpace : u32 {
    Register = 0u,
    Memory = 1u
};
enum class Function : u32 {
    Always = 0u,
    LessThan = 1u,
    LessThanEqual = 2u,
    Equal = 3u,
    NotEqual = 4u,
    GreaterThanEqual = 5u,
    GreaterThan = 6u,
    Reserved = 7u
};

}   // End namespace WaitRegMem

enum class PM4ItOpcode : u32 {
    Nop = 0x10,
    SetBase = 0x11,
    ClearState = 0x12,
    IndexBufferSize = 0x13,
    DispatchDirect = 0x15,
    DispatchIndirect = 0x16,
    AtomicGds = 0x1D,
    Atomic = 0x1E,
    OcclusionQuery = 0x1F,
    SetPredication = 0x20,
    RegRmw = 0x21,
    CondExec = 0x22,
    PredExec = 0x23,
    DrawIndirect = 0x24,
    DrawIndexIndirect = 0x25,
    IndexBase = 0x26,
    DrawIndex2 = 0x27,
    ContextControl = 0x28,
    IndexType = 0x2A,
    DrawIndirectMulti = 0x2C,
    DrawIndexAuto = 0x2D,
    NumInstances = 0x2F,
    DrawIndexMultiAuto = 0x30,
    IndirectBufferConst = 0x33,
    StrmoutBufferUpdate = 0x34,
    DrawIndexOffset2 = 0x35,
    WriteData = 0x37,
    DrawIndexIndirectMulti = 0x38,
    MemSemaphore = 0x39,
    WaitRegMem = 0x3c,
    IndirectBuffer = 0x3F,
    CondIndirectBuffer = 0x3F,
    CopyData = 0x40,
    CommandProcessorDma = 0x41,
    PfpSyncMe = 0x42,
    SurfaceSync = 0x43,
    CondWrite = 0x45,
    EventWrite = 0x46,
    EventWriteEop = 0x47,
    EventWriteEos = 0x48,
    ReleaseMem = 0x49,
    PreambleCntl = 0x4A,
    DmaData = 0x50,
    ContextRegRmw = 0x51,
    AcquireMem = 0x58,
    Rewind = 0x59,
    LoadShReg = 0x5F,
    LoadConfigReg = 0x60,
    LoadContextReg = 0x61,
    SetConfigReg = 0x68,
    SetContextReg = 0x69,
    SetContextRegIndirect = 0x73,
    SetShReg = 0x76,
    SetShRegOffset = 0x77,
    SetQueueReg = 0x78,
    SetUconfigReg = 0x79,
    LoadConstRam = 0x80,
    WriteConstRam = 0x81,
    DumpConstRam = 0x83,
    IncrementCeCounter = 0x84,
    IncrementDeCounter = 0x85,
    WaitOnCeCounter = 0x86,
    WaitOnDeCounterDiff = 0x88,
    GetLodStats = 0x8E,
    DrawIndexIndirectCountMulti = 0x9d,
};

}   // End namespace PS4::GCN