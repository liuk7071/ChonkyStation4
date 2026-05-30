#include "SceAjm.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAjm {

MAKE_LOG_FUNCTION(log, lib_sceAjm);

void init(Module& module) {
    module.addSymbolExport("eDFeTyi+G3Y", "sceAjmDecMp3ParseFrame", "libSceAjm", "libSceAjm", (void*)&sceAjmDecMp3ParseFrame);
    
    module.addSymbolStub("dl+4eHSzUu4", "sceAjmInitialize", "libSceAjm", "libSceAjm");
    module.addSymbolStub("Q3dyFuwGn64", "sceAjmModuleRegister", "libSceAjm", "libSceAjm");
    module.addSymbolStub("Wi7DtlLV+KI", "sceAjmModuleUnregister", "libSceAjm", "libSceAjm");
    module.addSymbolStub("AxoDrINp4J8", "sceAjmInstanceCreate", "libSceAjm", "libSceAjm");
    module.addSymbolStub("dmDybN--Fn8", "sceAjmBatchJobControlBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("7jdAXK+2fMo", "sceAjmBatchJobRunSplitBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("fFFkk0xfGWs", "sceAjmBatchStartBuffer", "libSceAjm", "libSceAjm");
    module.addSymbolStub("-qLsfDAywIY", "sceAjmBatchWait", "libSceAjm", "libSceAjm");
    module.addSymbolStub("MHur6qCsUus", "sceAjmFinalize", "libSceAjm", "libSceAjm");
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