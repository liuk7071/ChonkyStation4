#pragma once

#include <Common.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceAjm {

void init(Module& module);

using SceAjmContextId   = u32;
using SceAjmCodecType   = u32;
using SceAjmInstanceId  = u32;
using SceAjmBatchId     = u32;

static constexpr u32 SCE_AJM_CODEC_MP3_DEC = 0;

struct SceAjmInstance : SceObj {
    SceAjmCodecType codec = 0;
};

struct SceAjmBatchError;

enum SceAjmDecMp3OflType {
    SCE_AJM_DEC_MP3_OFL_TYPE_NONE           = 0,
    SCE_AJM_DEC_MP3_OFL_TYPE_LAME           = 1,
    SCE_AJM_DEC_MP3_OFL_TYPE_VBRI           = 2,
    SCE_AJM_DEC_MP3_OFL_TYPE_FGH            = 3,
    SCE_AJM_DEC_MP3_OFL_TYPE_VBRI_AND_FGH   = 4
};

struct SceAjmDecMp3ParseFrame {
    size_t frame_size;
    unsigned int n_channels;
    unsigned int samples_per_channel;
    unsigned int bitrate;
    unsigned int sample_rate;
    unsigned int ofl_encoder_delay;
    unsigned int ofl_num_frames;
    unsigned int ofl_total_samples;
    SceAjmDecMp3OflType ofl_type;
};

s32 PS4_FUNC sceAjmInstanceCreate(const SceAjmContextId ctx_id, const SceAjmCodecType codec, const u64 flags, SceAjmInstanceId* instance_id);
void* PS4_FUNC sceAjmBatchJobRunBufferRa(void* batch, const SceAjmInstanceId instance_id, const u64 flags, const void* input_data, const size_t input_data_size, void* output_data, const size_t output_data_size, void* sideband_output, const size_t sideband_output_size, const void* return_address);
s32 PS4_FUNC sceAjmBatchStartBuffer(const SceAjmContextId ctx_id, const void* batch, const size_t batch_size, const s32 prio, SceAjmBatchError* batch_error, SceAjmBatchId* batch_id);
s32 PS4_FUNC sceAjmDecMp3ParseFrame(const void* buf, size_t size, int parse_ofl, SceAjmDecMp3ParseFrame* frame_info);

}   // End namespace PS4::OS::Libs::SceAjm