#include "ScePlayGo.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceSystemService/SceSystemService.hpp>


namespace PS4::OS::Libs::ScePlayGo {

MAKE_LOG_FUNCTION(log, lib_scePlayGo);

void init(Module& module) {
    module.addSymbolExport("ts6GlZOKRrE", "scePlayGoInitialize", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoInitialize);
    module.addSymbolExport("M1Gma1ocrGE", "scePlayGoOpen", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoOpen);
    module.addSymbolExport("uWIYLFkkwqk", "scePlayGoGetLocus", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetLocus);
    module.addSymbolExport("-RJWNMK3fC8", "scePlayGoGetProgress", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetProgress);
    module.addSymbolExport("Nn7zKwnA5q0", "scePlayGoGetToDoList", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetToDoList);
    module.addSymbolExport("3OMbYZBaa50", "scePlayGoGetLanguageMask", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetLanguageMask);
    
    module.addSymbolStub("73fF1MFU8hA", "scePlayGoGetChunkId", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("-Q1-u1a7p0g", "scePlayGoPrefetch", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("LosLlHOpNqQ", "scePlayGoSetLanguageMask", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("gUPGiOQ1tmQ", "scePlayGoSetToDoList", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("4AAcTU9R3XM", "scePlayGoSetInstallSpeed", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("rvBSfTimejE", "scePlayGoGetInstallSpeed", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("v6EZ-YWRdMs", "scePlayGoGetEta", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("Uco1I0dlDi8", "scePlayGoClose", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("MPe0EeBGM-E", "scePlayGoTerminate", "libScePlayGo", "libScePlayGo");
}

ScePlayGoInitParams param;

s32 PS4_FUNC scePlayGoInitialize(const ScePlayGoInitParams* init_param) {
    log("scePlayGoInitialize(init_param=*%p)\n", init_param);
    param = *init_param;
    return SCE_OK;
}

s32 PS4_FUNC scePlayGoOpen(ScePlayGoHandle* out_handle, const void* param) {
    log("scePlayGoOpen(out_handle=*%p, param=%p)\n", out_handle, param);

    *out_handle = PLAYGO_HANDLE;
    return SCE_OK;
}

s32 PS4_FUNC scePlayGoGetLocus(ScePlayGoHandle handle, const ScePlayGoChunkId* chunk_ids, u32 n_entries, ScePlayGoLocus* out_loci) {
    log("scePlayGoGetLocus(handle=0x%x, chunk_ids=*%p, n_entries=%d, out_loci=*%p)\n", handle, chunk_ids, n_entries, out_loci);

    // TODO: Properly check playgo-chunk.dat
    for (int i = 0; i < n_entries; i++) {
        out_loci[i] = (ScePlayGoLocus)ScePlayGoLocusValue::LocalFast;
    }
    return SCE_OK;
}

s32 PS4_FUNC scePlayGoGetProgress(ScePlayGoHandle handle, const ScePlayGoChunkId* chunk_ids, u32 n_entries, ScePlayGoProgress* out_progress) {
    log("scePlayGoGetProgress(handle=0x%x, chunk_ids=*%p, n_entries=%d, out_progress=*%p)\n", handle, chunk_ids, n_entries, out_progress);

    // TODO
    out_progress->progress_size = 1_MB;
    out_progress->total_size = 1_MB;
    return SCE_OK;
}

s32 PS4_FUNC scePlayGoGetToDoList(ScePlayGoHandle handle, ScePlayGoToDo* out_todo_list, u32 n_entries, u32* n_out_entries) {
    log("scePlayGoGetToDoList(handle=0x%x, out_todo_list=*%p, n_entries=%d, n_out_entries=*%p)\n", handle, out_todo_list, n_entries, n_out_entries);

    *n_out_entries = 0;
    return SCE_OK;
}

inline ScePlayGoLanguageMask PS4_FUNC scePlayGoConvertLanguage(s32 system_lang) {
    return (system_lang >= 0 && system_lang < 48) ? (1ull << (64 - system_lang - 1)) : 0;
}

s32 PS4_FUNC scePlayGoGetLanguageMask(ScePlayGoHandle handle, ScePlayGoLanguageMask* out_mask) {
    log("scePlayGoGetLanguageMask(handle=0x%x, out_mask=*%p)\n", handle, out_mask);

    s32 lang = 0;
    SceSystemService::sceSystemServiceParamGetInt(SceSystemService::SCE_SYSTEM_SERVICE_PARAM_ID_LANG, &lang);
    *out_mask = scePlayGoConvertLanguage(lang);
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::ScePlayGo