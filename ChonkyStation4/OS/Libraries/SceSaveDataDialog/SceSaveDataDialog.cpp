#include "SceSaveDataDialog.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSaveDataDialog {

MAKE_LOG_FUNCTION(log, lib_sceSaveDataDialog);

s32 status;

void init(Module& module) {
    module.addSymbolExport("s9e3+YpRnzw", "sceSaveDataDialogInitialize", "libSceSaveDataDialog", "libSceSaveDataDialog", (void*)&sceSaveDataDialogInitialize);
    module.addSymbolExport("4tPhsP6FpDI", "sceSaveDataDialogOpen", "libSceSaveDataDialog", "libSceSaveDataDialog", (void*)&sceSaveDataDialogOpen);
    module.addSymbolExport("yEiJ-qqr6Cg", "sceSaveDataDialogGetResult", "libSceSaveDataDialog", "libSceSaveDataDialog", (void*)&sceSaveDataDialogGetResult);
    module.addSymbolExport("KK3Bdg1RWK0", "sceSaveDataDialogUpdateStatus", "libSceSaveDataDialog", "libSceSaveDataDialog", (void*)&sceSaveDataDialogUpdateStatus);
    module.addSymbolExport("YuH2FA7azqQ", "sceSaveDataDialogTerminate", "libSceSaveDataDialog", "libSceSaveDataDialog", (void*)&sceSaveDataDialogTerminate);
    

    status = 0; // Not running
}

s32 PS4_FUNC sceSaveDataDialogInitialize() {
    log("sceSaveDataDialogInitialize()\n");

    status = 1; // Initialized
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataDialogOpen(const SceSaveDataDialogParam* param) {
    log("sceSaveDataDialogOpen(param=*%p)\n", param);
    
    status = 2; // Running
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataDialogGetResult(SceSaveDataDialogResult* result) {
    log("sceSaveDataDialogGetResult(result=*%p)\n", result);
    
    if (status != 3) return SCE_COMMON_DIALOG_ERROR_NOT_FINISHED;

    result->result = SCE_COMMON_DIALOG_RESULT_OK;
    return result->result;
}

s32 PS4_FUNC sceSaveDataDialogUpdateStatus() {
    log("sceSaveDataDialogUpdateStatus()\n");
    log("status: %d\n", status);

    const s32 val = status;
    // Simulate the dialog being closed after it was opened
    if (status == 2)
        status = 3; // Finished
    return val;
}

s32 PS4_FUNC sceSaveDataDialogTerminate() {
    log("sceSaveDataDialogTerminate()\n");
    
    status = 0; // Not running
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceSaveDataDialog