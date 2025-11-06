cmake_minimum_required(VERSION 3.10)

project("ChonkyStation4")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(ChonkyStation4)
target_compile_definitions(ChonkyStation4 PUBLIC "_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
target_sources(ChonkyStation4 PRIVATE "ChonkyStation4/ChonkyStation4.cpp" "ChonkyStation4/Common/Common.hpp" "ChonkyStation4/Common/Logger.hpp" "ChonkyStation4/Loaders/ELF/ELFLoader.cpp" "ChonkyStation4/Loaders/ELF/ELFLoader.hpp" "ChonkyStation4/Loaders/ELF/CodePatcher.cpp" "ChonkyStation4/Loaders/ELF/CodePatcher.hpp" "ChonkyStation4/Loaders/Linker/Linker.cpp" "ChonkyStation4/Loaders/Linker/Linker.hpp" "ChonkyStation4/Loaders/Module.hpp" "ChonkyStation4/Loaders/Symbol.hpp" "ChonkyStation4/Loaders/App.cpp" "ChonkyStation4/Loaders/App.hpp" "ChonkyStation4/OS/HLE.cpp" "ChonkyStation4/OS/HLE.hpp" "ChonkyStation4/OS/Thread.cpp" "ChonkyStation4/OS/Thread.hpp" "ChonkyStation4/OS/SceObj.cpp" "ChonkyStation4/OS/SceObj.hpp" "ChonkyStation4/OS/Libraries/Kernel/Kernel.hpp" "ChonkyStation4/OS/Libraries/Kernel/Kernel.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/mutex.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/mutex.hpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/cond.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/cond.hpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/pthread.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/pthread.hpp" "ChonkyStation4/OS/Libraries/SceVideoOut/SceVideoOut.hpp" "ChonkyStation4/OS/Libraries/SceVideoOut/SceVideoOut.cpp")

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory(Dependencies/zydis)
add_subdirectory(Dependencies/pthread-win32)

target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4)
target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4/Common)

target_include_directories(ChonkyStation4 PUBLIC Dependencies/ELFIO)
target_include_directories(ChonkyStation4 PUBLIC Dependencies/xbyak)
target_include_directories(ChonkyStation4 PUBLIC Dependencies/pthread-win32)

target_link_libraries(ChonkyStation4 PRIVATE pthreadVC3 Zydis)

set_target_properties(ChonkyStation4 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)