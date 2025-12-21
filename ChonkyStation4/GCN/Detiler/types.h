#ifndef _GPUADDRESS_TYPES_H_
#define _GPUADDRESS_TYPES_H_

#include <stdbool.h>
#include <stddef.h>

#include "gnm/dataformat.h"
#include "gnm/types.h"

typedef enum {
	GPA_SURFACE_COLORDISPLAY,
	GPA_SURFACE_COLOR,
	GPA_SURFACE_DEPTHSTENCIL,
	GPA_SURFACE_DEPTH,
	GPA_SURFACE_STENCIL,
	GPA_SURFACE_FMASK,
	GPA_SURFACE_TEXTUREFLAT,
	GPA_SURFACE_TEXTUREVOLUME,
	GPA_SURFACE_TEXTURECUBEMAP,
	GPA_SURFACE_RWTEXTUREFLAT,
	GPA_SURFACE_RWTEXTUREVOLUME,
	GPA_SURFACE_RWTEXTURECUBEMAP,
} GpaSurfaceType;

typedef struct {
	uint32_t colortarget : 1;
	uint32_t depthtarget : 1;
	uint32_t stenciltarget : 1;
	uint32_t texture : 1;
	uint32_t cube : 1;
	uint32_t volume : 1;
	uint32_t fmask : 1;
	uint32_t cubeasarray : 1;
	uint32_t overlay : 1;
	uint32_t display : 1;
	uint32_t prt : 1;
	uint32_t pow2pad : 1;
	uint32_t texcompatible : 1;
	uint32_t _unused : 19;
} GpaSurfaceFlags;
_Static_assert(sizeof(GpaSurfaceFlags) == 0x4, "");

typedef struct {
	GnmTileMode tilemode;
	GpaSurfaceFlags flags;
} GpaSurfaceProperties;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t numslices;
	uint32_t numfrags;
	uint32_t bpp;

	GnmArrayMode arraymode;
	GnmNumBanks banks;
	GnmPipeConfig pipeconfig;

	GnmGpuMode mingpumode;

	struct {
		uint32_t tccompatible : 1;
		uint32_t reserved : 31;
	} flags;
} GpaHtileParams;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t numslices;
	uint32_t numfrags;
	uint32_t bpp;
	GnmTileMode tilemode;

	GnmGpuMode mingpumode;

	struct {
		uint32_t tccompatible : 1;
		uint32_t reserved : 31;
	} flags;
} GpaCmaskParams;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t numslices;
	uint32_t numfrags;
	uint32_t bpp;
	GnmTileMode tilemode;

	GnmGpuMode mingpumode;

	bool isblockcompressed;
} GpaFmaskParams;

typedef struct {
	GnmArrayMode arraymode;
	GnmNumBanks banks;
	GnmBankWidth bankwidth;
	GnmBankHeight bankheight;
	GnmMacroTileAspect macroaspectratio;
	GnmTileSplit tilesplit;
	GnmPipeConfig pipeconfig;
} GpaTileInfo;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t depth;
	uint64_t surfacesize;
	uint32_t basealign;
	uint32_t pitchalign;
	uint32_t heightalign;
	uint32_t depthalign;
	uint32_t bitsperelem;

	uint32_t blockwidth;
	uint32_t blockheight;

	GnmTileMode tilemode;
	GpaTileInfo tileinfo;

	struct {
		uint32_t istexcompatible : 1;
		uint32_t _unused : 31;
	};
} GpaSurfaceInfo;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t basealign;
	uint32_t bpp;
	uint32_t macrowidth;
	uint32_t macroheight;
	uint64_t htilebytes;
	uint64_t slicebytes;
} GpaHtileInfo;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t basealign;
	uint32_t bpp;
	uint32_t macrowidth;
	uint32_t macroheight;
	uint32_t blockmax;
	uint64_t cmaskbytes;
	uint64_t slicebytes;
} GpaCmaskInfo;

typedef struct {
	uint32_t pitch;
	uint32_t height;
	uint32_t basealign;
	uint32_t pitchalign;
	uint32_t heightalign;
	uint32_t bpp;
	uint64_t fmaskbytes;
	uint64_t slicebytes;
} GpaFmaskInfo;

typedef struct {
	uint32_t arrayindex;
	uint32_t face;
	uint32_t mip;
	uint32_t depth;
	uint32_t fragment;
	uint32_t sample;
} GpaSurfaceIndex;

typedef struct {
	GnmTileMode tilemode;
	GnmGpuMode mingpumode;

	uint32_t linearwidth;
	uint32_t linearheight;
	uint32_t lineardepth;
	uint32_t numfragsperpixel;
	uint32_t basetiledpitch;

	uint32_t miplevel;
	uint32_t arrayslice;
	GpaSurfaceFlags surfaceflags;
	uint32_t bitsperfrag;
	bool isblockcompressed;
} GpaTilingParams;

typedef struct {
	uint32_t left;	 // -X
	uint32_t top;	 // -Y
	uint32_t front;	 // -Z

	uint32_t right;	  // +X
	uint32_t bottom;  // +Y
	uint32_t back;	  // +Z
} GpaSurfaceRegion;

typedef struct {
	GnmGpuMode mingpumode;
	GnmTileMode tilemode;
	GpaTileInfo tileinfo;

	uint32_t linearwidth;
	uint32_t linearheight;
	uint32_t lineardepth;
	uint32_t paddedwidth;
	uint32_t paddedheight;
	uint32_t paddeddepth;

	uint32_t bitsperelement;
	uint32_t numfragsperpixel;

	uint32_t bankswizzlemask;
	uint32_t pipeswizzlemask;
} GpaSurfaceContext;

typedef struct {
	GnmTextureType type;
	GnmDataFormat fmt;

	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t depth;

	uint32_t numfrags;
	uint32_t nummips;
	uint32_t numslices;

	GnmTileMode tm;
	GnmGpuMode mingpumode;

	bool pow2pad;
} GpaTextureInfo;

#endif	// _GPUADDRESS_TYPES_H_
