#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceSaveDataDialog {

void init(Module& module);

static constexpr s32 SCE_COMMON_DIALOG_ERROR_NOT_FINISHED = 0x80B80005;

static constexpr s32 SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE = 32;
static constexpr s32 SCE_COMMON_DIALOG_RESULT_OK = 0;
static constexpr s32 SCE_COMMON_DIALOG_RESULT_USER_CANCELED = 1;

struct SceSaveDataDialogParam;
struct SceSaveDataParam;

enum class SceSaveDataDialogMode : u32 {
    Invalid = 0,
    List = 1,
    UserMsg = 2,
    SystemMsg = 3,
    ErrorCode = 4,
    ProgressBar = 5,
};

enum class SceSaveDataDialogButtonId : u32 {
    Invalid = 0,
    Ok = 1,
    Yes = 1,
    No = 2,
};

struct SceSaveDataDirName {
    char data[SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE];
};

struct SceSaveDataDialogResult {
    SceSaveDataDialogMode mode;
    s32 result;
    SceSaveDataDialogButtonId button_id;
    s32 : 32;
    SceSaveDataDirName* dir_name;
    SceSaveDataParam* param;
    void* userData;
    uint8_t reserved[32];
};

s32 PS4_FUNC sceSaveDataDialogInitialize();
s32 PS4_FUNC sceSaveDataDialogOpen(const SceSaveDataDialogParam* param);
s32 PS4_FUNC sceSaveDataDialogGetResult(SceSaveDataDialogResult* result);
s32 PS4_FUNC sceSaveDataDialogUpdateStatus();
s32 PS4_FUNC sceSaveDataDialogTerminate();

}   // End namespace PS4::OS::Libs::SceSaveDataDialog