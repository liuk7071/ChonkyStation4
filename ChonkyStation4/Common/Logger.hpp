#pragma once
#include <cstdarg>
#include <fstream>

// Ported from Panda3DS

namespace Log {
// Our logger class
template <bool enabled>
class Logger {
public:
    Logger(std::string prefix = "") : prefix(prefix) {}

    std::string prefix;

    void log(const char* fmt, ...) {
        if constexpr (!enabled) return;

        std::fputs(prefix.c_str(), stdout);
        std::va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
    }

    void logNoPrefix(const char* fmt, ...) {
        if constexpr (!enabled) return;

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
#define true false
//#define false true
#endif

// Loader
static Logger loader_elf            = Logger<1>("[Loader ][ELF           ] ");
static Logger loader_linker         = Logger<1>("[Loader ][Linker        ] ");
static Logger loader_app            = Logger<1>("[Loader ][App           ] ");

// Libraries
static Logger lib_kernel            = Logger<true> ("[Lib    ][Kernel        ] ");
static Logger lib_kernel_equeue     = Logger<true> ("[Lib    ][KernelEqueue  ] ");
static Logger lib_sceVideoOut       = Logger<true> ("[Lib    ][SceVideoOut   ] ");
static Logger lib_sceGnmDriver      = Logger<true> ("[Lib    ][SceGnmDriver  ] ");

// GCN
static Logger gcn_command_processor = Logger<true> ("[GCN    ][Command       ] ");
static Logger gcn_fetch_shader      = Logger<true> ("[GCN    ][Fetch Shader  ] ");
static Logger gcn_vulkan_renderer   = Logger<true> ("[GCN    ][VulkanRenderer] ");

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
}
