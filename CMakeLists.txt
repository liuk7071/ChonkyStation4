cmake_minimum_required(VERSION 3.10)

project("ChonkyStation4")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_definitions(SDL_MAIN_HANDLED)

add_executable(ChonkyStation4)
add_compile_definitions("_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
target_sources(ChonkyStation4 PRIVATE
"ChonkyStation4/ChonkyStation4.cpp" "ChonkyStation4/PlayStation4.cpp" "ChonkyStation4/PlayStation4.hpp" "ChonkyStation4/Common/Common.hpp" "ChonkyStation4/Common/Logger.hpp" "ChonkyStation4/Loaders/ELF/ELFLoader.cpp"
"ChonkyStation4/OS/Filesystem.cpp" "ChonkyStation4/OS/Filesystem.hpp"
"ChonkyStation4/Loaders/ELF/ELFLoader.hpp" "ChonkyStation4/Loaders/ELF/CodePatcher.cpp" "ChonkyStation4/Loaders/ELF/CodePatcher.hpp" "ChonkyStation4/Loaders/Linker/Linker.cpp" "ChonkyStation4/Loaders/Linker/Linker.hpp"
"ChonkyStation4/Loaders/App/AppLoader.cpp" "ChonkyStation4/Loaders/App/AppLoader.hpp" "ChonkyStation4/Loaders/SFO/SFOLoader.cpp" "ChonkyStation4/Loaders/SFO/SFOLoader.hpp" "ChonkyStation4/Loaders/Module.hpp"
"ChonkyStation4/Loaders/Symbol.hpp" "ChonkyStation4/Loaders/App.cpp" "ChonkyStation4/Loaders/App.hpp" "ChonkyStation4/GCN/PM4.hpp" "ChonkyStation4/GCN/CommandProcessor.cpp" "ChonkyStation4/GCN/CommandProcessor.hpp"
"ChonkyStation4/GCN/GCN.cpp" "ChonkyStation4/GCN/GCN.hpp" "ChonkyStation4/GCN/RegisterOffsets.hpp" "ChonkyStation4/GCN/FetchShader.cpp" "ChonkyStation4/GCN/FetchShader.hpp" "ChonkyStation4/GCN/Shader/Opcodes.hpp"
"ChonkyStation4/GCN/VSharp.hpp" "ChonkyStation4/GCN/TSharp.hpp"
"ChonkyStation4/GCN/Shader/Instruction.hpp" "ChonkyStation4/GCN/Shader/Instruction.cpp" "ChonkyStation4/GCN/Shader/Decoder.hpp" "ChonkyStation4/GCN/Shader/Decoder.cpp" "ChonkyStation4/GCN/Shader/Format.cpp" "ChonkyStation4/GCN/Shader/ShaderDecompiler.cpp"
"ChonkyStation4/GCN/Shader/ShaderDecompiler.hpp" "ChonkyStation4/GCN/Backends/Renderer.hpp" "ChonkyStation4/GCN/Backends/Vulkan/VulkanRenderer.cpp" "ChonkyStation4/GCN/Backends/Vulkan/VulkanRenderer.hpp" "ChonkyStation4/GCN/Backends/Vulkan/GLSLCompiler.hpp"
"ChonkyStation4/OS/HLE.cpp" "ChonkyStation4/OS/HLE.hpp" "ChonkyStation4/OS/Thread.cpp" "ChonkyStation4/OS/Thread.hpp" "ChonkyStation4/OS/SceObj.cpp" "ChonkyStation4/OS/SceObj.hpp" "ChonkyStation4/OS/Libraries/Kernel/Kernel.hpp"
"ChonkyStation4/OS/Libraries/Kernel/Kernel.cpp" "ChonkyStation4/OS/Libraries/Kernel/Equeue.hpp" "ChonkyStation4/OS/Libraries/Kernel/Equeue.cpp" "ChonkyStation4/OS/Libraries/Kernel/Filesystem.hpp" "ChonkyStation4/OS/Libraries/Kernel/Filesystem.cpp"
"ChonkyStation4/OS/Libraries/Kernel/pthread/mutex.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/mutex.hpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/cond.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/cond.hpp"
"ChonkyStation4/OS/Libraries/Kernel/pthread/pthread.cpp" "ChonkyStation4/OS/Libraries/Kernel/pthread/pthread.hpp" "ChonkyStation4/OS/Libraries/SceVideoOut/SceVideoOut.hpp" "ChonkyStation4/OS/Libraries/SceVideoOut/SceVideoOut.cpp"
"ChonkyStation4/OS/Libraries/SceGnmDriver/SceGnmDriver.cpp" "ChonkyStation4/OS/Libraries/SceGnmDriver/SceGnmDriver.hpp"
"ChonkyStation4/OS/Libraries/SceSystemService/SceSystemService.cpp" "ChonkyStation4/OS/Libraries/SceSystemService/SceSystemService.hpp"
"ChonkyStation4/OS/Libraries/SceNpManager/SceNpManager.cpp" "ChonkyStation4/OS/Libraries/SceNpManager/SceNpManager.hpp"
"ChonkyStation4/OS/Libraries/SceUserService/SceUserService.cpp" "ChonkyStation4/OS/Libraries/SceUserService/SceUserService.hpp"
"ChonkyStation4/OS/Libraries/SceSysmodule/SceSysmodule.cpp" "ChonkyStation4/OS/Libraries/SceSysmodule/SceSysmodule.hpp"
"ChonkyStation4/OS/Libraries/SceSaveData/SceSaveData.cpp" "ChonkyStation4/OS/Libraries/SceSaveData/SceSaveData.hpp"
"ChonkyStation4/OS/Libraries/SceNpTrophy/SceNpTrophy.cpp" "ChonkyStation4/OS/Libraries/SceNpTrophy/SceNpTrophy.hpp"
"ChonkyStation4/OS/Libraries/ScePad/ScePad.cpp" "ChonkyStation4/OS/Libraries/ScePad/ScePad.hpp"
"ChonkyStation4/OS/Libraries/SceAudioOut/SceAudioOut.cpp" "ChonkyStation4/OS/Libraries/SceAudioOut/SceAudioOut.hpp"
"ChonkyStation4/OS/Libraries/SceSaveDataDialog/SceSaveDataDialog.cpp" "ChonkyStation4/OS/Libraries/SceSaveDataDialog/SceSaveDataDialog.hpp"
 "ChonkyStation4/GCN/Backends/Vulkan/Pipeline.cpp" "ChonkyStation4/GCN/Backends/Vulkan/VulkanCommon.cpp" "ChonkyStation4/GCN/Backends/Vulkan/PipelineCache.cpp"
"ChonkyStation4/GCN/Detiler/decompress.c" "ChonkyStation4/GCN/Detiler/error.c" "ChonkyStation4/GCN/Detiler/surface.c" "ChonkyStation4/GCN/Detiler/surfgen.c" "ChonkyStation4/GCN/Detiler/tilemodes.c"
"ChonkyStation4/GCN/Detiler/tiler.c" "ChonkyStation4/GCN/Detiler/gnm/dataformat.c" "ChonkyStation4/GCN/Detiler/gnm/platform_generic.c" "ChonkyStation4/GCN/Detiler/gnm/texture.c"
 "ChonkyStation4/GCN/Backends/Vulkan/TextureCache.cpp" "ChonkyStation4/GCN/Backends/Vulkan/TextureCache.hpp" "ChonkyStation4/OS/Libraries/Kernel/Eflag.cpp" "ChonkyStation4/OS/Libraries/Kernel/Eflag.hpp"
 "ChonkyStation4/OS/Libraries/Kernel/Semaphore.cpp" "ChonkyStation4/OS/Libraries/Kernel/Semaphore.hpp" "ChonkyStation4/OS/Libraries/ScePlayGo/ScePlayGo.cpp" "ChonkyStation4/OS/Libraries/ScePlayGo/ScePlayGo.hpp"
 "ChonkyStation4/OS/UserManagement.cpp" "ChonkyStation4/OS/UserManagement.hpp")

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory(Dependencies/zydis)
if(WIN32)
    add_subdirectory(Dependencies/pthread-win32)
endif()
add_subdirectory(Dependencies/SDL)
add_subdirectory(Dependencies/xxHash/cmake_unofficial EXCLUDE_FROM_ALL)

set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
add_subdirectory(Dependencies/glslang)

target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4)
target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4/Common)

target_include_directories(ChonkyStation4 PUBLIC Dependencies/ELFIO)
target_include_directories(ChonkyStation4 PUBLIC Dependencies/xbyak)
if(WIN32)
    target_include_directories(ChonkyStation4 PUBLIC Dependencies/pthread-win32)
endif()
target_include_directories(ChonkyStation4 PUBLIC Dependencies/Dolphin)
target_include_directories(ChonkyStation4 PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(ChonkyStation4 PUBLIC Dependencies/xxHash)

if (WIN32)
    if (MSVC)
        target_link_options(ChonkyStation4 PRIVATE /DYNAMICBASE:NO)
        target_link_options(ChonkyStation4 PRIVATE /BASE:0x700000000000)
        target_link_options(ChonkyStation4 PRIVATE /STACK:0x200000,0x200000)
    else()
        target_link_options(ChonkyStation4 PRIVATE -Wl,--disable-dynamicbase)
        target_link_options(ChonkyStation4 PRIVATE -Wl,--image-base=0x700000000000)
        target_link_options(ChonkyStation4 PRIVATE -Wl,--stack,2097152)
    endif()
else()
    target_link_options(ChonkyStation4 PRIVATE -Wl,--image-base=0x700000000000)
endif()

if(WIN32)
    target_link_libraries(ChonkyStation4 PRIVATE pthreadVC3)
endif()
target_link_libraries(ChonkyStation4 PRIVATE Zydis SDL2-static xxHash::xxhash)

# Vulkan
find_package(Vulkan REQUIRED)
target_compile_definitions(ChonkyStation4 PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
target_link_libraries(ChonkyStation4 PRIVATE Vulkan::Vulkan glslang SPIRV)

set_target_properties(ChonkyStation4 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)