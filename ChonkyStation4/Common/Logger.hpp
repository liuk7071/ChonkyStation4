#pragma once
#include <cstdarg>
#include <fstream>
#include <mutex>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// Ported from Panda3DS

namespace Log {

inline std::mutex logger_mtx;

// Our logger class
template <bool enabled>
class Logger {
public:
    Logger(std::string prefix = "") : prefix(prefix) {}

    std::string prefix;

    void log(const char* fmt, ...) {
        if constexpr (!enabled) return;

        const std::lock_guard<std::mutex> lock(logger_mtx);
        std::fputs(prefix.c_str(), stdout);
#ifdef _WIN32
        PWSTR thread_name;
        GetThreadDescription(GetCurrentThread(), &thread_name);
        std::wprintf(L"(%ls) ", thread_name);
#endif
        std::va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
    }

    void logNoPrefix(const char* fmt, ...) {
        if constexpr (!enabled) return;

        const std::lock_guard<std::mutex> lock(logger_mtx);
        std::va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
    }
};

// Our loggers here. Enable/disable by toggling the template param

#ifdef CHONKYSTATION4_USER_BUILD
#define true false
#else
//#define true false
//#define false true
#endif

// Loader
static Logger loader_elf            = Logger<0>    ("[Loader ][ELF              ] ");
static Logger loader_linker         = Logger<0>    ("[Loader ][Linker           ] ");
static Logger loader_sfo            = Logger<1>    ("[Loader ][SFO              ] ");
static Logger loader_app            = Logger<1>    ("[Loader ][App              ] ");

// Libraries
static Logger lib_kernel            = Logger<true> ("[Lib    ][Kernel           ] ");
static Logger lib_kernel_mutex      = Logger<false>("[Lib    ][KernelMutex      ] ");
static Logger lib_kernel_equeue     = Logger<true> ("[Lib    ][KernelEqueue     ] ");
static Logger lib_kernel_eflag      = Logger<true> ("[Lib    ][KernelEflag      ] ");
static Logger lib_kernel_filesystem = Logger<true> ("[Lib    ][KernelFilesys    ] ");
static Logger lib_sceVideoOut       = Logger<true> ("[Lib    ][SceVideoOut      ] ");
static Logger lib_sceGnmDriver      = Logger<true> ("[Lib    ][SceGnmDriver     ] ");
static Logger lib_sceSystemService  = Logger<true> ("[Lib    ][SceSystemService ] ");
static Logger lib_sceUserService    = Logger<true> ("[Lib    ][SceUserService   ] ");
static Logger lib_sceNpManager      = Logger<true> ("[Lib    ][SceNpManager     ] ");
static Logger lib_sceSaveData       = Logger<true> ("[Lib    ][SceSaveData      ] ");
static Logger lib_sceSaveDataDialog = Logger<true> ("[Lib    ][SceSaveDataDialog] ");
static Logger lib_sceNpTrophy       = Logger<true> ("[Lib    ][SceNpTrophy      ] ");
static Logger lib_scePad            = Logger<true> ("[Lib    ][ScePad           ] ");
static Logger lib_sceAudioOut       = Logger<true> ("[Lib    ][SceAudioOut      ] ");

// GCN
static Logger gcn_command_processor = Logger<true> ("[GCN    ][Command          ] ");
static Logger gcn_fetch_shader      = Logger<true> ("[GCN    ][Fetch Shader     ] ");
static Logger gcn_vulkan_renderer   = Logger<true> ("[GCN    ][VulkanRenderer   ] ");

// Other
static Logger filesystem            = Logger<true> ("[Other  ][Filesystem       ] ");
static Logger unimplemented         = Logger<true> ("[Other  ][Unimplemented    ] ");

#undef true
#undef false

// We have 2 ways to create a log function
// MAKE_LOG_FUNCTION: Creates a log function which is toggleable but always killed for user-facing builds
// MAKE_LOG_FUNCTION_USER: Creates a log function which is toggleable, may be on for user builds as well
// We need this because sadly due to the loggers taking variadic arguments, compilers will not properly
// Kill them fully even when they're disabled. The only way they will is if the function with varargs is totally empty

#define MAKE_LOG_FUNCTION_USER(functionName, logger)                    \
	template <typename... Args>                                         \
	void functionName(const char* fmt, Args&&... args) {                \
		Log::logger.log(fmt, args...);                             \
	}                                                                   \
    template <typename... Args>                                         \
    void functionName##NoPrefix(const char* fmt, Args&&... args) {      \
        Log::logger.logNoPrefix(fmt, args...);                     \
    }

#ifdef CHONKYSTATION4_USER_BUILD
#define MAKE_LOG_FUNCTION(functionName, logger)             \
	template <typename... Args>                             \
	void functionName(const char* fmt, Args&&... args) {}   \
    template <typename... Args>                             \
	void functionName##NoPrefix(const char* fmt, Args&&... args) {}
#else
#define MAKE_LOG_FUNCTION(functionName, logger) MAKE_LOG_FUNCTION_USER(functionName, logger)
#endif

}   // End namespace Log
