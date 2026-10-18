// #include "SceMbus.hpp"
// #include <Logger.hpp>
// #include <Loaders/Module.hpp>
// #include <deque>
// #include <mutex>
// #include <semaphore>
//
//
// namespace PS4::OS::Libs::SceMbus {
//
// MAKE_LOG_FUNCTION(log, lib_sceMbus);
//
// void init(Module& module) {
//     module.addSymbolExport("puHrnP8V-dY", "sceMbusEventReceive", "libSceMbus", "libSceMbus", (void*)&sceMbusEventReceive);
//     module.addSymbolExport("KRL-S9qBqXw", "sceMbusGetDeviceInfoByCondition_", "libSceMbus", "libSceMbus", (void*)&sceMbusGetDeviceInfoByCondition_);
//
//     module.addSymbolStub("wRPXMGtkOq0", "sceMbusInit", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("c08SEHicDNU", "sceMbusEventCreate_", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("HgPSJ1kcnHM", "sceMbusEventCallbackFuncsInit_", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("0LkfqnKtPQg", "sceMbusEventCreate", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("edYHYROxzx4", "libSceMbus_edYHYROxzx4", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("wpm6Yq7c4YE", "sceMbusSetAutoLoginMode", "libSceMbus", "libSceMbus");
//     module.addSymbolStub("Sq1DqijPveA", "sceMbusSetScratchDataUpdatedEventMask", "libSceMbus", "libSceMbus");
//
//     auto login_thr = std::thread([&]() {
//         std::this_thread::sleep_for(std::chrono::seconds(20));
//         submitEvent({ .system = 1, .event_id = 1, .device_id = 100 });
//     });
//     login_thr.detach();
// }
//
// std::mutex queue_mtx;
// std::counting_semaphore<256> queue_sem { 0 };
// std::deque<SceMbusEvent> events;
// void submitEvent(const SceMbusEvent& event) {
//     const std::unique_lock<std::mutex> lk(queue_mtx);
//     events.push_back(event);
//     queue_sem.release();
// }
//
// SceMbusEvent readEvent() {
//     queue_sem.acquire();
//
//     SceMbusEvent event;
//     {
//         const std::unique_lock<std::mutex> lk(queue_mtx);
//         event = events.front();
//         events.pop_front();
//     }
//     return event;
// }
//
// s32 PS4_FUNC sceMbusEventReceive(u32 handle, SceMbusEvent* out_events, u32 n_events, u32 unk1 /* maybe timeout? */) {
//     log("sceMbusEventReceive(handle=%d, out_events=*%p, n_events=%d, unk1=%d)\n", handle, out_events, n_events, unk1);
//
//     if (n_events > 1)
//         printf("sceMbusEventReceive: n_events > 1");
//
//     out_events[0] = readEvent();
//     return 1;
// }
//
// struct SceMbusDeviceInfo {
//     u64 qword[32];
// };
//
// s32 PS4_FUNC sceMbusGetDeviceInfoByCondition_(u32* unk1, u64 unk2, void* unk3, u64 unk4, u64* out_n_devices, SceMbusDeviceInfo* out_info, u64 unk7) {
//     log("sceMbusGetDeviceInfoByCondition_(unk1=%p, unk2=%lld, unk3=%p, unk4=%lld, out_n_devices=*%p, out_info=%p, unk7=%lld)\n", unk1, unk2, unk3, unk4, out_n_devices, out_info, unk7);
//
//     // *unk1: uid?
//     // unk2 don't know
//     // unk3 don't know
//     // unk4 type?
//     // unk6 don't know
//     // unk7 don't know
//
//     log("uid: %d\n", *unk1);
//     log("*unk3: *%d\n", *(u32*)unk3);
//
//     //out_info[0].device_unique_id    = 1;
//     out_info[0].qword[0] = 0x1;
//     *(u32*)((u8*)&out_info[0] + 0x0C) = 2;
//     *(u32*)((u8*)&out_info[0] + 0xC8) = 0;
//
//     const auto n_devices = 1;
//     *out_n_devices = 1;
//     return 1;
// }
//
// }   // End namespace PS4::OS::Libs::SceMbus

#include "SceMbus.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/Kernel/pthread/pthread.hpp>
#include <OS/Libraries/ScePad/ScePad.hpp>
#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

namespace PS4::OS::Libs::SceMbus {

MAKE_LOG_FUNCTION(log, lib_sceMbus);

namespace {
constexpr s32 invalid_argument = static_cast<s32>(0x809b0080u);
constexpr s32 cancelled = static_cast<s32>(0x809b0088u);
struct EventQueue {
    std::mutex mutex;
    std::condition_variable changed;
    bool closed = false;
    u32 mask;
    size_t capacity;
    std::deque<SceMbusEvent> events;
    EventReceived received = nullptr;
    void* user = nullptr;
    void* worker = nullptr;
};
std::mutex bus_mutex;
std::array<std::shared_ptr<EventQueue>, 8> queues;
s32 controller_user = 0;
s32 device_generation = 1;
bool auto_login = false;

SceMbusDeviceInfo controllerInfo() {
    SceMbusDeviceInfo info {};


    info.device_id = ScePad::device_id;
    info.transport = 0;
    info.type = 2;
    info.function_count = 1;
    info.functions[0] = {0, 0, 0, 0};
    std::lock_guard lock(bus_mutex);
    info.user_id = controller_user;
    return info;
}
struct CallbackContext { u32 handle; std::shared_ptr<EventQueue> queue; };
void* PS4_FUNC dispatchCallbacks(void* arg) {
    std::unique_ptr<CallbackContext> context(static_cast<CallbackContext*>(arg));
    const auto& queue = context->queue;
    std::unique_lock lock(queue->mutex);
    for (;;) {
        queue->changed.wait(lock, [&] { return queue->closed || !queue->events.empty(); });
        if (queue->closed) return nullptr;
        const auto event = queue->events.front();
        queue->events.pop_front();
        lock.unlock();
        queue->received(context->handle, queue->user, &event);
        lock.lock();
    }
}
s32 PS4_FUNC sceMbusInit() { return SCE_OK; }
}

void init(Module& module) {
    module.addSymbolExport("puHrnP8V-dY", "sceMbusEventReceive", "libSceMbus", "libSceMbus", (void*)&sceMbusEventReceive);
    module.addSymbolExport("KRL-S9qBqXw", "sceMbusGetDeviceInfoByCondition_", "libSceMbus", "libSceMbus", (void*)&sceMbusGetDeviceInfoByCondition_);
    module.addSymbolExport("wRPXMGtkOq0", "sceMbusInit", "libSceMbus", "libSceMbus", (void*)&sceMbusInit);
    module.addSymbolExport("c08SEHicDNU", "sceMbusEventCreate_", "libSceMbus", "libSceMbus", (void*)&sceMbusEventCreate_);
    module.addSymbolExport("HgPSJ1kcnHM", "sceMbusEventCallbackFuncsInit_", "libSceMbus", "libSceMbus", (void*)&sceMbusEventCallbackFuncsInit_);
    module.addSymbolExport("edYHYROxzx4", "sceMbusDeviceInfoInit", "libSceMbus", "libSceMbus", (void*)&sceMbusDeviceInfoInit);
    module.addSymbolExport("RsKVKhSWt28", "sceMbusGetDeviceInfo", "libSceMbus", "libSceMbus", (void*)&sceMbusGetDeviceInfo);
    module.addSymbolExport("Cpo6sQmnZgU", "sceMbusBindDeviceWithUserId", "libSceMbus", "libSceMbus", (void*)&sceMbusBindDeviceWithUserId);
    module.addSymbolExport("wxmyc+71ijI", "sceMbusConvertToLocalDeviceId2", "libSceMbus", "libSceMbus", (void*)&sceMbusConvertToLocalDeviceId2);
    module.addSymbolExport("wpm6Yq7c4YE", "sceMbusSetAutoLoginMode", "libSceMbus", "libSceMbus", (void*)&sceMbusSetAutoLoginMode);
    module.addSymbolExport("AcARz3iay1I", "sceMbusEventDestroy", "libSceMbus", "libSceMbus", (void*)&sceMbusEventDestroy);
    module.addSymbolStub("0LkfqnKtPQg", "sceMbusEventCreate", "libSceMbus", "libSceMbus");
    module.addSymbolStub("Sq1DqijPveA", "sceMbusSetScratchDataUpdatedEventMask", "libSceMbus", "libSceMbus");
}

void submitEvent(const SceMbusEvent& event) {
    std::lock_guard lock(bus_mutex);
    for (const auto& queue : queues) {
        if (!queue || (queue->mask & event.type) == 0) continue;
        {
            std::lock_guard queue_lock(queue->mutex);
            if (queue->closed) continue;
            if (queue->events.size() == queue->capacity) queue->events.pop_front();
            queue->events.push_back(event);
        }
        queue->changed.notify_all();
    }
}

s32 PS4_FUNC sceMbusEventCreate_(u32 mask, void* buffer, s32 capacity,
    const SceMbusEventCallbacks* callbacks, void* user) {
    if (!mask || (buffer && capacity <= 0)) return invalid_argument;
    auto queue = std::make_shared<EventQueue>();
    queue->mask = mask;
    queue->capacity = buffer ? static_cast<size_t>(capacity) : 32;
    queue->received = callbacks ? callbacks->received : nullptr;
    queue->user = user;
    u32 handle = 0;
    {
        std::lock_guard lock(bus_mutex);
        while (handle < queues.size() && queues[handle]) ++handle;
        if (handle == queues.size()) return invalid_argument;
        queues[handle] = queue;
    }
    if (queue->received) {
        auto* context = new CallbackContext {handle, queue};

        const auto result = Kernel::scePthreadCreate(&queue->worker, nullptr,
            dispatchCallbacks, context, "SceMbusEventCallback");
        if (result != SCE_OK) {
            delete context;
            std::lock_guard lock(bus_mutex);
            queues[handle].reset();
            return result;
        }
    }


    if (mask & 1) {
        std::lock_guard lock(queue->mutex);
        queue->events.push_back({1, 0, ScePad::device_id, {}});
        queue->changed.notify_all();
    }
    return static_cast<s32>(handle);
}

s32 PS4_FUNC sceMbusEventReceive(u32 handle, SceMbusEvent* out_events, u32 n_events, u32) {
    if (!out_events || !n_events) return invalid_argument;
    std::shared_ptr<EventQueue> queue;
    {
        std::lock_guard lock(bus_mutex);
        if (handle >= queues.size() || !(queue = queues[handle])) return invalid_argument;
    }
    if (queue->received) return invalid_argument;
    std::unique_lock lock(queue->mutex);
    queue->changed.wait(lock, [&] { return queue->closed || !queue->events.empty(); });
    if (queue->closed) return cancelled;
    const auto count = std::min<size_t>(n_events, queue->events.size());
    for (size_t i = 0; i < count; ++i) {
        out_events[i] = queue->events.front();
        queue->events.pop_front();
    }
    return static_cast<s32>(count);
}

s32 PS4_FUNC sceMbusEventDestroy(u32 handle) {
    std::shared_ptr<EventQueue> queue;
    {
        std::lock_guard lock(bus_mutex);
        if (handle >= queues.size() || !(queue = queues[handle])) return invalid_argument;
        std::lock_guard queue_lock(queue->mutex);
        queue->closed = true;
        queues[handle].reset();
    }
    queue->changed.notify_all();
    if (queue->worker) {
        if (Kernel::kernel_pthread_equal(queue->worker, Kernel::kernel_pthread_self()))
            Kernel::kernel_pthread_detach(queue->worker);
        else
            Kernel::kernel_pthread_join(queue->worker, nullptr);
    }
    return SCE_OK;
}

void PS4_FUNC sceMbusDeviceInfoInit(void* data, u32 size) {
    if (!data) return;
    std::memset(data, 0, size);
    if (size >= 16) std::memset(data, 0xff, 16);
    if (size >= 204) {
        const s32 any_user = -1;
        std::memcpy(static_cast<u8*>(data) + 200, &any_user, sizeof(any_user));
    }
}
void PS4_FUNC sceMbusEventCallbackFuncsInit_(void* callbacks, u32 size) {
    if (callbacks) std::memset(callbacks, 0, size);
}

s32 PS4_FUNC sceMbusGetDeviceInfoByCondition_(const SceMbusDeviceInfo* condition, s32 start,
    SceMbusDeviceInfo* out_info, s32 capacity, s32* total, s32* generation, s32 stride) {
    if (!condition || start < 0 || capacity < 0 || (!out_info && capacity) ||
        stride != sizeof(SceMbusDeviceInfo) || condition->function_count < 0 || condition->function_count > 11)
        return invalid_argument;
    const auto device = controllerInfo();
    bool functions_match = true;
    for (s32 i = 0; i < condition->function_count; ++i) {
        const auto& requested = condition->functions[i];
        bool found = false;
        for (s32 j = 0; j < device.function_count; ++j) {
            const auto& available = device.functions[j];
            if ((requested.type == -1 || requested.type == available.type) &&
                (requested.state == -1 || requested.state == available.state)) found = true;
        }
        functions_match &= found;
    }
    const bool match = functions_match &&
        (condition->device_id == UINT64_MAX || condition->device_id == device.device_id) &&
        (condition->transport == -1 || condition->transport == device.transport) &&
        (condition->type == -1 || condition->type == device.type) &&
        (condition->user_id == -1 || condition->user_id == device.user_id);
    if (total) *total = match ? 1 : 0;
    if (generation) {
        std::lock_guard lock(bus_mutex);
        *generation = device_generation;
    }
    const s32 count = match && start == 0 && capacity > 0 ? 1 : 0;
    if (count) *out_info = device;
    log("sceMbusGetDeviceInfoByCondition_: type=%d owner=%d -> %d\n", condition->type, condition->user_id, count);
    return count;
}

s32 PS4_FUNC sceMbusGetDeviceInfo(u64 id, SceMbusDeviceInfo* info, u32 size) {
    if (id != ScePad::device_id || !info || size != sizeof(*info)) return invalid_argument;
    *info = controllerInfo();
    return SCE_OK;
}

s32 PS4_FUNC sceMbusBindDeviceWithUserId(u64 id, s32 user) {
    if (id != ScePad::device_id || user < 0) return invalid_argument;
    {
        std::lock_guard lock(bus_mutex);
        if (controller_user == user) return SCE_OK;
        controller_user = user;
        ++device_generation;
    }
    SceMbusEvent event {0x10, 0, id, {}};
    const auto info = controllerInfo();
    std::memcpy(event.payload, &info.transport, sizeof(info.transport));
    std::memcpy(event.payload + 4, &info.type, sizeof(info.type));
    std::memcpy(event.payload + 12, &user, sizeof(user));
    submitEvent(event);
    return SCE_OK;
}

s32 PS4_FUNC sceMbusConvertToLocalDeviceId2(u64 id, u32 bus, u64* local) {
    if (id != ScePad::device_id || bus != 0 || !local) return invalid_argument;
    *local = id;
    return SCE_OK;
}
s32 PS4_FUNC sceMbusSetAutoLoginMode(u32 mode) {
    if (mode > 1) return invalid_argument;
    std::lock_guard lock(bus_mutex);
    auto_login = mode != 0;
    return SCE_OK;
}

}
