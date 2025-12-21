#ifndef _GPU_ADDRESS_H_
#define _GPU_ADDRESS_H_

#include "gnm/types.h"

#include "error.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Surface
//
GpaError gpaComputeSurfaceInfo(GpaSurfaceInfo* out, const GpaTilingParams* tp);
GpaError gpaComputeHtileInfo(
    GpaHtileInfo* outinfo, const GpaHtileParams* params
);
GpaError gpaComputeCmaskInfo(
    GpaCmaskInfo* outinfo, const GpaCmaskParams* params
);
GpaError gpaComputeFmaskInfo(
    GpaFmaskInfo* outinfo, const GpaFmaskParams* params
);
GpaError gpaComputeSurfaceTileMode(
    GnmTileMode* outtilemode, GnmGpuMode mingpumode, GnmArrayMode arraymode,
    GpaSurfaceFlags flags, GnmDataFormat surfacefmt, uint32_t numfragsperpixel,
    GnmMicroTileMode mtm
);

GpaError gpaInitSurfaceContext(
    GpaSurfaceContext* ctx, size_t surfsize, const GpaTilingParams* tp
);
GpaError gpaComputeSurfaceCoord(
    uint64_t* outoffset, uint64_t* outbitoffset, const GpaSurfaceContext* ctx,
    uint32_t x, uint32_t y, uint32_t z, uint32_t fragindex
);
GpaError gpaComputeSurfaceSizeOffset(
    uint64_t* outsize, uint64_t* outoffset, const GpaTextureInfo* tex,
    uint32_t miplevel, uint32_t arrayslice
);

//
// Surface generation
//
GpaError gpaFindOptimalSurface(
    GpaSurfaceProperties* outprops, GpaSurfaceType surfacetype, uint32_t bpp,
    uint32_t numfrags, bool mipmapped, GnmGpuMode mingpumode
);

//
// Element/Utility
//

GpaError gpaGetTileInfo(
    GpaTileInfo* outinfo, GnmTileMode tilemode, uint32_t bpp, uint32_t numfrags,
    GnmGpuMode gpumode
);
GpaError gpaComputeBaseSwizzle(
    uint32_t* outswizzle, GnmTileMode tilemode, uint32_t surfindex,
    uint32_t bpp, uint32_t numfrags, GnmGpuMode gpumode
);

//
// Decompression
//
GpaError gpaGetDecompressedSize(
    uint64_t* outlen, const GpaTextureInfo* texinfo
);
GpaError gpaDecompressTexture(
    void* outbuf, uint64_t outlen, const void* inbuf, uint64_t inlen,
    const GpaTextureInfo* texinfo, GnmDataFormat* outfmt
);

//
// Tiler
//
GpaError gpaTpInit(
    GpaTilingParams* tp, const GpaTextureInfo* tex, uint32_t miplevel,
    uint32_t arrayslice
);

GpaError gpaTileSurface(
    void* outbuf, size_t outlen, const void* inbuf, size_t inlen,
    const GpaTilingParams* srctp, const GpaTilingParams* dst_tp
);
GpaError gpaTileSurfaceRegion(
    void* outbuf, size_t outlen, const void* inbuf, size_t inlen,
    const GpaTilingParams* srctp, const GpaTilingParams* dst_tp,
    const GpaSurfaceRegion* region
);
GpaError gpaTileTextureIndexed(
    const void* inbuf, size_t inlen, void* outbuf, size_t outlen,
    const GpaTextureInfo* texinfo, GnmTileMode newtiling, uint32_t mip,
    uint32_t slice
);
GpaError gpaTileTextureAll(
    const void* inbuf, size_t inlen, void* outbuf, size_t outlen,
    const GpaTextureInfo* texinfo, GnmTileMode newtiling
);

#ifdef __cplusplus
}
#endif

#endif	// _GPU_ADDRESS_H_
