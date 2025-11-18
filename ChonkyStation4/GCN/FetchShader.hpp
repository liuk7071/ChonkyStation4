#pragma once

#include <Common.hpp>


class VSharp;

namespace PS4::GCN {

struct VSharpLocation {
    u32 sgpr = 0;   // SGPR pair that contains the pointer to the V#
    u32 offs = 0;   // Offset in DWORDs from the pointer above

    VSharp* asPtr();
};

struct VertexBinding {
    VSharpLocation vsharp_loc;
    u32 dest_vgpr = 0;
    u32 n_elements = 0; // Elements per attrib (i.e. 4 for xyzw)
    u32 soffs = 0;
    u32 voffs = 0;
    u32 inst_offs = 0;
};

class FetchShader {
public:
    FetchShader(const u8* data);
    std::vector<VertexBinding> bindings;
};

}   // End namespace PS4::GCN