#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceNpWebApi {

void init(Module& module);

static constexpr s32 SCE_NP_WEBAPI_ERROR_REQUEST_NOT_FOUND = 0x80552906;

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

s32 PS4_FUNC sceNpWebApiCreateRequest(s32 user_ctx_id, const char* api_group, const char* path, SceNpWebApiHttpMethod method, const SceNpWebApiContentParameter* content_parameter, s64* req_id);
s32 PS4_FUNC sceNpWebApiSendRequest2(s64 req_id, const void* data, size_t data_size, SceNpWebApiResponseInformationOption* error_response);
s32 PS4_FUNC sceNpWebApiReadData(s64 req_id, void* data, size_t size);

}   // End namespace PS4::OS::Libs::SceNpWebApi