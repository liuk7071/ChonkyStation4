#include "HostTessShaders.hpp"
#include <Common.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>


namespace PS4::GCN::HostShaders {

void generateRectTCS(Shader::ShaderData* vs_data, std::string& out_str) {
    // We setup tessellation to pass in 3 control points.
    // In the Tessellation Control Shader, we passthrough the first 3 points, and then generate the fourth by interpolating all the attributes.
    // The missing vertex is the top-right one. The input vertices are, in order: bottom-left, bottom-right, top-left.

    out_str.reserve(16_KB);
    out_str = R"(
#version 450

layout(vertices = 4) out;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("layout(location = {}) in  vec4 in_attr{}[];\n", out, out);
        out_str += std::format("layout(location = {}) out vec4 out_attr{}[];\n\n", out, out);
    }

    out_str +=
R"(void main() {
    if (gl_InvocationID < 3) {
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;        
)";

    // Passthrough attributes 0 to 2
    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("    out_attr{}[gl_InvocationID] = in_attr{}[gl_InvocationID];\n", out, out);
    }

    out_str +=
R"(    } else {
        vec4 v0 = gl_in[2].gl_Position;
        vec4 v1 = gl_in[0].gl_Position;
        vec4 v2 = gl_in[1].gl_Position;
        gl_out[gl_InvocationID].gl_Position = v0 + (v2 - v1);
)";

    // Do linear interpolation of all attributes for the missing vertex
    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("    out_attr{}[gl_InvocationID] = in_attr{}[2] + (in_attr{}[1] - in_attr{}[0]);\n", out, out, out, out);
    }

    out_str +=
R"(    }

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1.0f;
        gl_TessLevelOuter[1] = 1.0f;
        gl_TessLevelOuter[2] = 1.0f;
		gl_TessLevelOuter[3] = 1.0f;
        gl_TessLevelInner[0] = 1.0f;
		gl_TessLevelInner[1] = 1.0f;
    }
}
)";
}

void generateRectTES(Shader::ShaderData* vs_data, std::string& out_str) {
    // For a quad, gl_TessCoords are as follows:
    // bottom-left:  0, 0
    // bottom-right: 1, 0
    // top-left:     0, 1
    // top-right:    1, 1
    // So we can calculate the vertex "index" by doing y * 2 + x, which maps back to GCN vertex order (see the TCS above).
    
    out_str.reserve(16_KB);
    out_str = R"(
#version 450

layout(quads, equal_spacing, ccw) in;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("layout(location = {}) in  vec4 in_attr{}[];\n", out, out);
        out_str += std::format("layout(location = {}) out vec4 out_attr{};\n\n", out, out);
    }

    out_str +=
R"(void main() {
    const int idx = int(gl_TessCoord.y) * 2 + int(gl_TessCoord.x);
    gl_Position = gl_in[idx].gl_Position;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("    out_attr{} = in_attr{}[idx];\n", out, out);
    }

    out_str += "}\n";
}

void generateQuadTCS(Shader::ShaderData* vs_data, std::string& out_str) {
    // For quads, we just pass everything through,
    // but we must reorder the vertices in the layout that Vulkan expects.

    out_str.reserve(16_KB);
    out_str = R"(
#version 450

const int reorder[4] = int[](1, 2, 0, 3);

layout(vertices = 4) out;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("layout(location = {}) in  vec4 in_attr{}[];\n", out, out);
        out_str += std::format("layout(location = {}) out vec4 out_attr{}[];\n\n", out, out);
    }

    out_str +=
        R"(void main() {
    const int id = reorder[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[id].gl_Position;        
)";

    // Passthrough attributes 0 to 2
    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("    out_attr{}[gl_InvocationID] = in_attr{}[id];\n", out, out);
    }

    out_str +=
        R"(
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1.0f;
        gl_TessLevelOuter[1] = 1.0f;
        gl_TessLevelOuter[2] = 1.0f;
		gl_TessLevelOuter[3] = 1.0f;
        gl_TessLevelInner[0] = 1.0f;
		gl_TessLevelInner[1] = 1.0f;
    }
}
)";
}

void generateQuadTES(Shader::ShaderData* vs_data, std::string& out_str) {
    // This is essentially the same as the Rect TES
    
    // For a quad, gl_TessCoords are as follows:
    // bottom-left:  0, 0
    // bottom-right: 1, 0
    // top-left:     0, 1
    // top-right:    1, 1
    // So we can calculate the vertex "index" by doing y * 2 + x, which maps back to GCN vertex order (see the TCS above).

    out_str.reserve(16_KB);
    out_str = R"(
#version 450

layout(quads, equal_spacing, ccw) in;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("layout(location = {}) in  vec4 in_attr{}[];\n", out, out);
        out_str += std::format("layout(location = {}) out vec4 out_attr{};\n\n", out, out);
    }

    out_str +=
        R"(void main() {
    const int idx = int(gl_TessCoord.y) * 2 + int(gl_TessCoord.x);
    gl_Position = gl_in[idx].gl_Position;
)";

    for (auto& out : vs_data->vtx_outputs) {
        out_str += std::format("    out_attr{} = in_attr{}[idx];\n", out, out);
    }

    out_str += "}\n";
}

}   // End namespace PS4::GCN::HostShaders