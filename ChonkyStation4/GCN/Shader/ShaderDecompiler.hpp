#pragma once

#include <Common.hpp>


class FetchShader;

namespace PS4::GCN::Shader {

enum class ShaderStage {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    Tessellation
};

enum class DescriptorType {
    Vsharp,
    Tsharp
};

struct BufferDescriptorInfo {
    int sgpr = -1;          // The user data SGPR the descriptor is in
    bool is_ptr = false;    // Whether or not the user data is a pointer to it or is the descriptor itself
    DescriptorType type;
};

struct Buffer {
    int binding = -1;
    BufferDescriptorInfo desc_info;
};

struct ShaderData {
    std::string source;
    
};

ShaderData decompileShader(u32* data, ShaderStage stage, FetchShader* fetch_shader = nullptr);

}   // End namespace PS4::GCN::Shader