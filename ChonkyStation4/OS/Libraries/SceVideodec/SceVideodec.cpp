#include "SceVideodec.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceVideodec {

MAKE_LOG_FUNCTION(log, lib_sceVideodec);

void init(Module& module) {
    // libSceVideodec
    module.addSymbolExport("leCAscipfFY", "sceVideodecQueryResourceInfo", "libSceVideodec", "libSceVideodec", (void*)&sceVideodecQueryResourceInfo);
    
    module.addSymbolStub("qkgRiwHyheU", "sceVideodecCreateDecoder", "libSceVideodec", "libSceVideodec");
    module.addSymbolStub("q0W5GJMovMs", "sceVideodecDecode", "libSceVideodec", "libSceVideodec");
    module.addSymbolStub("f8AgDv-1X8A", "sceVideodecReset", "libSceVideodec", "libSceVideodec");
    module.addSymbolStub("U0kpGF1cl90", "sceVideodecDeleteDecoder", "libSceVideodec", "libSceVideodec");
    
    // libSceVideodec2
    module.addSymbolExport("qqMCwlULR+E", "sceVideodec2QueryDecoderMemoryInfo", "libSceVideodec2", "libSceVideodec2", (void*)&sceVideodec2QueryDecoderMemoryInfo);
    
    module.addSymbolStub("RnDibcGCPKw", "sceVideodec2QueryComputeMemoryInfo", "libSceVideodec2", "libSceVideodec2");
    module.addSymbolStub("eD+X2SmxUt4", "sceVideodec2AllocateComputeQueue", "libSceVideodec2", "libSceVideodec2");
    module.addSymbolStub("CNNRoRYd8XI", "sceVideodec2CreateDecoder", "libSceVideodec2", "libSceVideodec2");
    module.addSymbolStub("852F5+q6+iM", "sceVideodec2Decode", "libSceVideodec2", "libSceVideodec2");
    module.addSymbolStub("wJXikG6QFN8", "sceVideodec2Reset", "libSceVideodec2", "libSceVideodec2");
    module.addSymbolStub("jwImxXRGSKA", "sceVideodec2DeleteDecoder", "libSceVideodec2", "libSceVideodec2");
}

// This function sets required memory sizes according to cfg_info in resource_info and returns.
s32 PS4_FUNC sceVideodecQueryResourceInfo(const SceVideodecConfigInfo* cfg_info, SceVideodecResourceInfo* resource_info) {
    log("sceVideodecQueryResourceInfo(cfg_info=*%p, resource_info=*%p) TODO\n", cfg_info, resource_info);

    // TODO
    resource_info->cpu_mem_size = 1920 * 1080 * 4;
    resource_info->cpu_gpu_mem_size = 1920 * 1080 * 4;
    resource_info->max_framebuffer_size = 1920 * 1080 * 4;
    resource_info->framebuffer_alignment = 16_KB;
    return SCE_OK;
}

// This function sets required memory sizes according to cfg_info in mem_info and returns.
s32 PS4_FUNC sceVideodec2QueryDecoderMemoryInfo(const SceVideodec2DecoderConfigInfo* cfg_info, SceVideodec2DecoderMemoryInfo* mem_info) {
    log("sceVideodec2QueryDecoderMemoryInfo(cfg_info=*%p, mem_info=*%p) TODO\n");

    // TODO
    mem_info->cpu_mem_size = 1920 * 1080 * 4;
    mem_info->gpu_mem_size = 1920 * 1080 * 4;
    mem_info->cpu_gpu_mem_size = 1920 * 1080 * 4;
    mem_info->max_framebuffer_size = 1920 * 1080 * 4;
    mem_info->framebuffer_alignment = 16_KB;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceVideodec