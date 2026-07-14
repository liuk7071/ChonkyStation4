#include "SceNpWebApi.hpp"
#include <httplib.h>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/SceObj.hpp>


namespace PS4::OS::Libs::SceNpWebApi {

MAKE_LOG_FUNCTION(log, lib_sceNpWebApi);

void init(Module& module) {
    module.addSymbolExport("rdgs5Z1MyFw", "sceNpWebApiCreateRequest", "libSceNpWebApi", "libSceNpWebApi", (void*)&sceNpWebApiCreateRequest);
    module.addSymbolExport("KjNeZ-29ysQ", "sceNpWebApiSendRequest2", "libSceNpWebApi", "libSceNpWebApi", (void*)&sceNpWebApiSendRequest2);
    module.addSymbolExport("CQtPRSF6Ds8", "sceNpWebApiReadData", "libSceNpWebApi", "libSceNpWebApi", (void*)&sceNpWebApiReadData);

    module.addSymbolStub("G3AnLNdRBjE", "sceNpWebApiInitialize", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("79M-JqvvGo0", "sceNpWebApiCreateHandle", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("x1Y7yiYSk7c", "sceNpWebApiCreateContext", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("zk6c65xoyO0", "sceNpWebApiCreateContextA", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("y5Ta5JCzQHY", "sceNpWebApiCreatePushEventFilter", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("gVNNyxf-1Sg", "sceNpWebApiCheckTimeout", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("qWcbJkBj1Lg", "sceNpWebApiSetRequestTimeout", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("M2BUB+DNEGE", "sceNpWebApiCreateExtdPushEventFilter", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("PfSTDCgNMgc", "sceNpWebApiRegisterPushEventCallback", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("jhXKGQJ4egI", "sceNpWebApiRegisterExtdPushEventCallbackA", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("PqCY25FMzPs", "sceNpWebApiUnregisterExtdPushEventCallback", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("pfaJtb7SQ80", "sceNpWebApiDeleteExtdPushEventFilter", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("noQgleu+KLE", "sceNpWebApiDeleteRequest", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("5Mn7TYwpl30", "sceNpWebApiDeleteHandle", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("XUjdsSTTZ3U", "sceNpWebApiDeleteContext", "libSceNpWebApi", "libSceNpWebApi");
}

struct SceNpWebApiRequest : SceObj {
    std::string server;
    std::string path;
    std::string url;
    SceNpWebApiHttpMethod method;

    std::string response_body;
    u64 read_cursor = 0;
    u64 to_read = 0;
};

s32 PS4_FUNC sceNpWebApiCreateRequest(s32 user_ctx_id, const char* api_group, const char* path, SceNpWebApiHttpMethod method, const SceNpWebApiContentParameter* content_parameter, s64* req_id) {
    log("sceNpWebApiCreateRequest(user_ctx_id=%d, api_group=\"%s\", path=\"%s\", method=%d, content_parameter=*%p)\n", user_ctx_id, api_group, path, method, content_parameter);
    
    auto* req = OS::make<SceNpWebApiRequest>();

    // TODO: Make server configurable
    const std::string server_addr = "http://127.0.0.1";
    req->server = server_addr;
    req->path = std::format("/{}{}", api_group, path);
    req->url = std::format("{}{}", server_addr, req->path);
    req->method = method;
    
    *req_id = req->handle;
    return 0;
}

s32 PS4_FUNC sceNpWebApiSendRequest2(s64 req_id, const void* data, size_t data_size, SceNpWebApiResponseInformationOption* error_response) {
    log("sceNpWebApiSendRequest2(req_id=%lld, data=*%p, data_size=%lld, error_response=*%p)\n", req_id, data, data_size, error_response);

    auto* req = OS::find<SceNpWebApiRequest>(req_id);
    if (!req) {
        log("request could not be found\n");
        return SCE_NP_WEBAPI_ERROR_REQUEST_NOT_FOUND;
    }

    auto method_to_str = [](SceNpWebApiHttpMethod method) -> std::string {
        switch (method) {
        case SCE_NP_WEBAPI_HTTP_METHOD_GET:     return "GET";
        case SCE_NP_WEBAPI_HTTP_METHOD_POST:    return "POST";
        case SCE_NP_WEBAPI_HTTP_METHOD_PUT:     return "PUT";
        case SCE_NP_WEBAPI_HTTP_METHOD_DELETE:  return "DELETE";
        default:    Helpers::panic("sceNpWebApiSendRequest2: http method %d is invalid\n", method);
        }
    };

    log("Sending WebApi request %s %s\n", method_to_str(req->method).c_str(), req->url.c_str());

    httplib::Client http(req->server);

    // TODO: Other methods
    if (auto res = http.Get(req->path)) {
        log("HTTP status code: %d\n", res->status);
        log("HTTP Response body:\n%s\n", res->body.c_str());
        req->response_body = res->body;
        req->read_cursor = 0;
        req->to_read = req->response_body.size();
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpWebApiReadData(s64 req_id, void* data, size_t size) {
    log("sceNpWebApiReadData(req_id=%lld, data=%p, size=%lld)\n", req_id, data, size);

    auto* req = OS::find<SceNpWebApiRequest>(req_id);
    if (!req) {
        log("request could not be found\n");
        return SCE_NP_WEBAPI_ERROR_REQUEST_NOT_FOUND;
    }

    const auto to_read = std::min(size, req->to_read);
    if (to_read > 0) {
        std::memcpy(data, req->response_body.c_str() + req->read_cursor, to_read);
        req->to_read -= to_read;
        req->read_cursor += to_read;
    }

    log("read %d bytes\n", to_read);
    return to_read;
}

}   // End namespace PS4::OS::Libs::SceNpWebApi