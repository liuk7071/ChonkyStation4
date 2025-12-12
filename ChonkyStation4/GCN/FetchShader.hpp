#pragma once

#include <Common.hpp>


class VSharp;

namespace PS4::GCN {

struct VSharpLocation {
    u32 sgpr = 0;   // SGPR pair that contains the pointer to the V#
    u32 offs = 0;   // Offset in DWORDs from the pointer above

    VSharp* asPtr();

    bool operator==(const VSharpLocation& other) {
        return sgpr == other.sgpr && offs == other.offs;
    }
};

struct FetchShaderVertexBinding {
    VSharpLocation vsharp_loc;
    u32 dest_vgpr = 0;
    u32 n_elements = 0; // Elements per attrib (i.e. 4 for xyzw)
    u32 soffs = 0;
    u32 voffs = 0;
    u32 inst_offs = 0;

    bool operator==(const FetchShaderVertexBinding& other) {
        return  vsharp_loc == other.vsharp_loc
            &&  dest_vgpr == other.dest_vgpr
            &&  n_elements == other.n_elements
            &&  soffs == other.soffs
            &&  voffs == other.voffs
            &&  inst_offs == other.inst_offs;
    }
};

class FetchShader {
public:
    FetchShader(const u8* data);
    std::vector<FetchShaderVertexBinding> bindings;

    bool operator==(const FetchShader& other) {
        if (bindings.size() != other.bindings.size()) return false;

        for (int i = 0; i < bindings.size(); i++) {
            if (bindings[i] != other.bindings[i]) return false;
        }
        return true;
    }
};

}   // End namespace PS4::GCN