#include "SceAjm.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAjm {

MAKE_LOG_FUNCTION(log, lib_sceAjm);

s32 PS4_FUNC sceAjmBatchWait() {
    //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return SCE_OK;
}

void init(Module& module) {
    module.addSymbolExport("AxoDrINp4J8", "sceAjmInstanceCreate", "libSceAjm", "libSceAjm", (void*)&sceAjmInstanceCreate);
    module.addSymbolExport("eDFeTyi+G3Y", "sceAjmDecMp3ParseFrame", "libSceAjm", "libSceAjm", (void*)&sceAjmDecMp3ParseFrame);
    module.addSymbolExport("ElslOCpOIns", "sceAjmBatchJobRunBufferRa", "libSceAjm", "libSceAjm", (void*)&sceAjmBatchJobRunBufferRa);
    module.addSymbolExport("fFFkk0xfGWs", "sceAjmBatchStartBuffer", "libSceAjm", "libSceAjm", (void*)&sceAjmBatchStartBuffer);
    module.addSymbolExport("-qLsfDAywIY", "sceAjmBatchWait", "libSceAjm", "libSceAjm", (void*)&sceAjmBatchWait);
    
    module.addSymbolStub("dl+4eHSzUu4", "sceAjmInitialize", "libSceAjm", "libSceAjm");
    module.addSymbolStub("Q3dyFuwGn64", "sceAjmModuleRegister", "libSceAjm", "libSceAjm");
    module.addSymbolStub("Wi7DtlLV+KI", "sceAjmModuleUnregister", "libSceAjm", "libSceAjm");
    module.addSymbolStub("dmDybN--Fn8", "sceAjmBatchJobControlBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("7jdAXK+2fMo", "sceAjmBatchJobRunSplitBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("RbLbuKv8zho", "sceAjmInstanceDestroy", "libSceAjm", "libSceAjm");
    module.addSymbolStub("MHur6qCsUus", "sceAjmFinalize", "libSceAjm", "libSceAjm");
}

struct BatchJob {
    SceAjmInstanceId instance_id = 0;
    u64 flags;
    void* input_data = nullptr;
    size_t input_size = 0;
    void* output_data = nullptr;
    size_t output_size = 0;
};

s32 PS4_FUNC sceAjmInstanceCreate(const SceAjmContextId ctx_id, const SceAjmCodecType codec, const u64 flags, SceAjmInstanceId* instance_id) {
    log("sceAjmInstanceCreate(ctx_id=%d, codec=%d, flags=0x%llx, instance_id=*%p)\n", ctx_id, codec, flags, instance_id);
    
    auto* instance = OS::make<SceAjmInstance>();
    instance->codec = codec;

    if (instance->handle > UINT32_MAX)
        Helpers::panic("sceAjmInstanceCreate: SceObj handle is too big (%d)\n", instance->handle);
    *instance_id = instance->handle;
    return SCE_OK;
}

void* PS4_FUNC sceAjmBatchJobRunBufferRa(void* batch, const SceAjmInstanceId instance_id, const u64 flags, const void* input_data, const size_t input_data_size, void* output_data, const size_t output_data_size, void* sideband_output, const size_t sideband_output_size, const void* return_address) {
    log("sceAjmBatchJobRunBufferRa(batch=%p, instance_id=%d, flags=0x%llx, input_data=%p, input_data_size=%lld, output_data=%p, output_data_size=%lld, sideband_output=%p, sideband_output_size=%lld, return_address=%p)\n", batch, instance_id, flags, input_data, input_data_size, output_data, output_data_size, sideband_output, sideband_output_size, return_address);

    auto* job = (BatchJob*)batch;
    job->instance_id = instance_id;
    job->flags = flags;
    job->input_data = (void*)input_data;
    job->input_size = input_data_size;
    job->output_data = (void*)output_data;
    job->output_size = output_data_size;

    std::memset(output_data, 10, output_data_size);
    std::memset(sideband_output, 0, sideband_output_size);
    return (void*)((uptr)batch + sizeof(BatchJob));
}

s32 PS4_FUNC sceAjmBatchStartBuffer(const SceAjmContextId ctx_id, const void* batch, const size_t batch_size, const s32 prio, SceAjmBatchError* batch_error, SceAjmBatchId* batch_id) {
    log("sceAjmBatchStartBuffer(ctx_id=%d, batch=%p, batch_size=%lld, prio=%d, batch_error=*%p, batch_id=*%p)\n", ctx_id, batch, batch_size, prio, batch_error, batch_id);

    auto* job = (BatchJob*)batch;
    auto* instance = OS::find<SceAjmInstance>(job->instance_id);
    //if (!instance)
    //    Helpers::panic("sceAjmBatchStartBuffer: job->instance_id is invalid\n");

    *batch_id = 1;
    return SCE_OK;
}

s32 PS4_FUNC sceAjmDecMp3ParseFrame(const void* buf, size_t size, int parse_ofl, SceAjmDecMp3ParseFrame* frame_info) {
    log("sceAjmDecMp3ParseFrame(buf=%p, size=%lld, parse_ofl=%d, frame_info=*%p) TODO\n", buf, size, parse_ofl, frame_info);

    // TODO
    std::memset(frame_info, 0, sizeof(SceAjmDecMp3ParseFrame));
    frame_info->frame_size = sizeof(float) * 2;
    frame_info->n_channels = 2;
    frame_info->samples_per_channel = 1;
    frame_info->bitrate = 1;
    frame_info->sample_rate = 48000;

    if (parse_ofl) {
        frame_info->ofl_encoder_delay = 1;
        frame_info->ofl_num_frames = 1;
        frame_info->ofl_total_samples = 2;
        frame_info->ofl_type = SceAjmDecMp3OflType::SCE_AJM_DEC_MP3_OFL_TYPE_LAME;
    }
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceAjm