#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::ScePlayGo {

void init(Module& module);

static constexpr s32 SCE_PLAYGO_ERROR_NOT_SUPPORT_PLAYGO = 0x80B2000E;

using ScePlayGoHandle = u32;
using ScePlayGoChunkId = u16;
using ScePlayGoLocus = u8;
static constexpr ScePlayGoHandle PLAYGO_HANDLE = 0x12345678;

struct ScePlayGoInitParams {
    const void* buf;
    u32 buf_size;
    u32 reserved;
};

struct ScePlayGoToDo {
    ScePlayGoChunkId chunk_id;
    ScePlayGoLocus locus;
    s8 reserved;
};

struct ScePlayGoProgress {
    u64 progress_size;
    u64 total_size;
};

enum class ScePlayGoLocusValue {
    NotDownloaded = 0,
    LocalSlow = 2,
    LocalFast = 3
};

s32 PS4_FUNC scePlayGoInitialize(const ScePlayGoInitParams* init_param);
s32 PS4_FUNC scePlayGoOpen(ScePlayGoHandle* out_handle, const void* param);
s32 PS4_FUNC scePlayGoGetLocus(ScePlayGoHandle handle, const ScePlayGoChunkId* chunk_ids, u32 n_entries, ScePlayGoLocus* out_loci);
s32 PS4_FUNC scePlayGoGetProgress(ScePlayGoHandle handle, const ScePlayGoChunkId* chunk_ids, u32 n_entries, ScePlayGoProgress* out_progress);
s32 PS4_FUNC scePlayGoGetToDoList(ScePlayGoHandle handle, ScePlayGoToDo* out_todo_list, u32 n_entries, u32* n_out_entries);

}   // End namespace PS4::OS::Libs::ScePlayGo