cmake_minimum_required(VERSION 3.10)

project("ChonkyStation4")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(ChonkyStation4)
target_sources(ChonkyStation4 PRIVATE "ChonkyStation4/ChonkyStation4.cpp" "ChonkyStation4/Common/Common.hpp" "ChonkyStation4/Common/Logger.hpp" "ChonkyStation4/Loaders/ELF/ELFLoader.cpp" "ChonkyStation4/Loaders/ELF/ELFLoader.hpp" "ChonkyStation4/Loaders/Linker/Linker.cpp" "ChonkyStation4/Loaders/Linker/Linker.hpp" "ChonkyStation4/Loaders/Module.hpp" "ChonkyStation4/Loaders/App.cpp" "ChonkyStation4/Loaders/App.hpp")

target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4)
target_include_directories(ChonkyStation4 PUBLIC ChonkyStation4/Common)

target_include_directories(ChonkyStation4 PUBLIC Dependencies/ELFIO)

set_target_properties(ChonkyStation4 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)