#include "ScePlayGo.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::ScePlayGo {

MAKE_LOG_FUNCTION(log, lib_scePlayGo);

void init(Module& module) {
    module.addSymbolExport("ts6GlZOKRrE", "scePlayGoInitialize", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoInitialize);
    module.addSymbolExport("M1Gma1ocrGE", "scePlayGoOpen", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoOpen);
    module.addSymbolStub("LosLlHOpNqQ", "scePlayGoSetLanguageMask", "libScePlayGo", "libScePlayGo");
    module.addSymbolExport("uWIYLFkkwqk", "scePlayGoGetLocus", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetLocus);
    module.addSymbolExport("Nn7zKwnA5q0", "scePlayGoGetToDoList", "libScePlayGo", "libScePlayGo", (void*)&scePlayGoGetToDoList);
    
    module.addSymbolStub("gUPGiOQ1tmQ", "scePlayGoSetToDoList", "libScePlayGo", "libScePlayGo");
    module.addSymbolStub("-RJWNMK3fC8", "scePlayGoGetProgress", "libScePlayGo", "libScePlayGo");
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

s32 PS4_FUNC scePlayGoGetToDoList(ScePlayGoHandle handle, ScePlayGoToDo* out_todo_list, u32 n_entries, u32* n_out_entries) {
    log("scePlayGoGetToDoList(handle=0x%x, out_todo_list=*%p, n_entries=%d, n_out_entries=*%p)\n", handle, out_todo_list, n_entries, n_out_entries);

    *n_out_entries = 0;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::ScePlayGo