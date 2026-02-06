#pragma once

#include <Common.hpp>
#include <GCN/GCN.hpp>


namespace PS4::GCN::Shader {

struct ShaderData;

}   // End namespace PS4::GCN::Shader

namespace PS4::GCN::HostShaders {

void generateRectTCS(Shader::ShaderData* vs_data, std::string& out_str);
void generateRectTES(Shader::ShaderData* vs_data, std::string& out_str);
void generateQuadTCS(Shader::ShaderData* vs_data, std::string& out_str);
void generateQuadTES(Shader::ShaderData* vs_data, std::string& out_str);

}   // End namespace PS4::GCN::HostShaders