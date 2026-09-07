#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <optional>


class Module;

namespace PS4::OS::Libs::SceNpWebApi {

void init(Module& module);

using namespace OS::Np;

static constexpr s32 SCE_NP_WEBAPI_ERROR_REQUEST_NOT_FOUND = 0x80552906;

static constexpr s32 SCE_NP_WEBAPI_PUSH_EVENT_DATA_TYPE_LEN_MAX = 64;

enum SceNpWebApiHttpMethod {
    SCE_NP_WEBAPI_HTTP_METHOD_GET,
    SCE_NP_WEBAPI_HTTP_METHOD_POST,
    SCE_NP_WEBAPI_HTTP_METHOD_PUT,
    SCE_NP_WEBAPI_HTTP_METHOD_DELETE
};

struct SceNpWebApiContentParameter {
    size_t content_length;
    const char* content_type;
    u8 reserved[16];
};

struct SceNpWebApiResponseInformationOption {
    s32 http_status;
    char* error_obj;
    size_t error_obj_size;
    size_t response_data_size;
};

struct SceNpWebApiPushEventDataType {
    char data[SCE_NP_WEBAPI_PUSH_EVENT_DATA_TYPE_LEN_MAX + 1];
};

using SceNpWebApiPushEventCallback = PS4_FUNC void (*)(const s32 user_ctx_id, s32 cb_id, const SceNpPeerAddress* to, const SceNpPeerAddress* from, const SceNpWebApiPushEventDataType* data_type, const char* data, size_t data_size, void* userdata);

void sendPushEvent(const std::string& data_type, const std::optional<SceNpPeerAddress> to, const std::optional<SceNpPeerAddress> from, const std::optional<std::string> data);
void checkCallback();

s32 PS4_FUNC sceNpWebApiCreateContext(s32 lib_ctx_id, SceUserService::SceUserServiceUserId uid);
s32 PS4_FUNC sceNpWebApiCreateRequest(s32 user_ctx_id, const char* api_group, const char* path, SceNpWebApiHttpMethod method, const SceNpWebApiContentParameter* content_parameter, s64* req_id);
s32 PS4_FUNC sceNpWebApiSendRequest(s64 req_id, const void* data, size_t data_size);
s32 PS4_FUNC sceNpWebApiSendRequest2(s64 req_id, const void* data, size_t data_size, SceNpWebApiResponseInformationOption* error_response);
s32 PS4_FUNC sceNpWebApiGetHttpStatusCode(s64 req_id, s32* status_code);
s32 PS4_FUNC sceNpWebApiReadData(s64 req_id, void* data, size_t size);
s32 PS4_FUNC sceNpWebApiCreatePushEventFilter(s32 lib_ctx_id, const SceNpWebApiPushEventDataType* data_type, size_t n_data_types);
s32 PS4_FUNC sceNpWebApiRegisterPushEventCallback(s32 user_ctx_id, s32 filter_id, SceNpWebApiPushEventCallback cb_func, void* userdata);
s32 PS4_FUNC sceNpWebApiUtilityParseNpId(const char* json_np_id, SceNpId* np_id);

}   // End namespace PS4::OS::Libs::SceNpWebApi