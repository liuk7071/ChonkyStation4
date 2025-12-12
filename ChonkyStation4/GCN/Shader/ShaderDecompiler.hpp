#pragma once

#include <Common.hpp>
#include <deque>


namespace PS4::GCN {

class FetchShader;

}   // End namespace PS4::GCN

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

// TODO: Duplicated from FetchShader.hpp
struct DescriptorLocation {
    u32 sgpr = 0;   // SGPR pair that contains the pointer to the descriptor or the descriptor itself
    u32 offs = 0;   // Offset in DWORDs from the pointer above
    bool is_ptr = false;    // Whether or not the user data is a pointer to it or is the descriptor itself
    DescriptorType type;
    ShaderStage stage;

    template<typename T> T* asPtr();
};

struct Buffer {
    int binding = -1;
    DescriptorLocation desc_info;
};

struct ShaderData {
    std::string source;
    std::deque<Buffer> buffers; // Buffers required by this shader

    bool operator==(const ShaderData& other) {
        return source == other.source;
    }
};

void decompileShader(u32* data, ShaderStage stage, ShaderData& out_data, FetchShader* fetch_shader = nullptr);

}   // End namespace PS4::GCN::Shader