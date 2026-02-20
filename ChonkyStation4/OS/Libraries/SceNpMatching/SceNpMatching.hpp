#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpMatching {

void init(Module& module);

static constexpr s32 SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID = 0x80550c0b;

using namespace OS::Np;
using SceNpMatching2ContextId       = u16;
using SceNpMatching2Event           = u16;
using SceNpMatching2EventCause      = u8;
using SceNpMatching2ServerId        = u16;
using SceNpMatching2RequestId       = u32;

using SceNpMatching2RequestCallback = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2RequestId req_id, SceNpMatching2Event event, s32 error_code, const void* data, void* arg);
using SceNpMatching2ContextCallback = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2Event event, SceNpMatching2EventCause event_cause, s32 error_code, void* arg);

struct SceNpMatching2RequestOptParam {
    SceNpMatching2RequestCallback req_callback;
    void* req_callback_arg;
    u32 timeout;
    u16 app_req_id;
    unsigned char padding[2];
};

struct SceNpMatching2CreateContextParam {
    SceNpId* np_id;
    void* np_communication_id;
    void* np_passphrase;
    SceNpServiceLabel service_label;
    u64 size;
};

struct SceNpMatching2GetWorldInfoListRequest {
    SceNpMatching2ServerId server_id;
};

struct SceNpMatching2Context : SceObj {
    SceNpMatching2CreateContextParam param = {};
    SceNpMatching2ServerId server_id = 0;
};

void checkCallback();

s32 PS4_FUNC sceNpMatching2CreateContext(const SceNpMatching2CreateContextParam* param, SceNpMatching2ContextId* ctx_id);
s32 PS4_FUNC sceNpMatching2RegisterContextCallback(SceNpMatching2ContextCallback callback, void* arg);
s32 PS4_FUNC sceNpMatching2ContextStart(const SceNpMatching2ContextId ctx_id, const u64 timeout);
s32 PS4_FUNC sceNpMatching2GetServerId(const SceNpMatching2ContextId ctx_id, SceNpMatching2ServerId* server_id);

}   // End namespace PS4::OS::Libs::SceNpMatching