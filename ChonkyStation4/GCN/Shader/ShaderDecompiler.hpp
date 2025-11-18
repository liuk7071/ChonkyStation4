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

std::string decompileShader(u32* data, ShaderStage stage, FetchShader* fetch_shader = nullptr);

}   // End namespace PS4::GCN::Shader