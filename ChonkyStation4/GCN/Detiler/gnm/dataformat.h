#ifndef _GNM_DATAFORMAT_H_
#define _GNM_DATAFORMAT_H_

#include <stdbool.h>

#include "types.h"

typedef union {
	struct {
		GnmImageFormat surfacefmt : 8;
		GnmImgNumFormat chantype : 4;
		GnmChannel chanx : 3;
		GnmChannel chany : 3;
		GnmChannel chanz : 3;
		GnmChannel chanw : 3;
		uint32_t _unused : 8;
	};
	uint32_t asuint;
} GnmDataFormat;
_Static_assert(sizeof(GnmDataFormat) == 0x4, "");

GnmDataFormat gnmDfInitFromFmask(uint32_t numsamples, uint32_t numfrags);
GnmDataFormat gnmDfInitFromZ(GnmZFormat zfmt);

static inline GnmDataFormat gnmDfInitFromStencil(
    GnmStencilFormat stencilfmt, GnmImgNumFormat chantype
) {
	GnmDataFormat res = {
	    .surfacefmt = stencilfmt == GNM_STENCIL_8
			      ? GNM_IMG_DATA_FORMAT_8
			      : GNM_IMG_DATA_FORMAT_INVALID,
	    .chantype = chantype,
	    .chanx = GNM_CHAN_X,
	    .chany = GNM_CHAN_X,
	    .chanz = GNM_CHAN_X,
	    .chanw = GNM_CHAN_X,
	};
	return res;
}

static inline uint32_t gnmDfGetTexelsPerElement(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_BC1:
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
	case GNM_IMG_DATA_FORMAT_BC4:
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_BC7:
		return 16;
	case GNM_IMG_DATA_FORMAT_1:
	case GNM_IMG_DATA_FORMAT_1_REVERSED:
		return 8;
	default:
		return 1;
	}
}

uint32_t gnmDfGetNumComponents(const GnmDataFormat datafmt);
uint32_t gnmDfGetBitsPerElement(const GnmDataFormat datafmt);
static inline uint32_t gnmDfGetTotalBitsPerElement(const GnmDataFormat fmt) {
	const uint32_t bitsperelem = gnmDfGetBitsPerElement(fmt);
	const uint32_t texelsperelem = gnmDfGetTexelsPerElement(fmt);
	return bitsperelem * texelsperelem;
}
static inline uint32_t gnmDfGetBytesPerElement(const GnmDataFormat datafmt) {
	return gnmDfGetBitsPerElement(datafmt) / 8;
}
static inline uint32_t gnmDfGetTotalBytesPerElement(const GnmDataFormat fmt) {
	return gnmDfGetTotalBitsPerElement(fmt) / 8;
}
static inline bool gnmDfIsBlockCompressed(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_BC1:
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
	case GNM_IMG_DATA_FORMAT_BC4:
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_BC7:
		return true;
	default:
		return false;
	}
}

bool gnmDfGetRtChannelType(const GnmDataFormat datafmt, GnmSurfaceNumber* out);
bool gnmDfGetRtChannelOrder(const GnmDataFormat datafmt, GnmSurfaceSwap* out);

GnmZFormat gnmDfGetZFormat(const GnmDataFormat datafmt);
GnmStencilFormat gnmDfGetStencilFormat(const GnmDataFormat datafmt);

static inline uint32_t gnmDfGetTexelsPerElementWide(const GnmDataFormat fmt) {
	switch (fmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_BC1:
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
	case GNM_IMG_DATA_FORMAT_BC4:
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_BC7:
		return 4;
	case GNM_IMG_DATA_FORMAT_1:
	case GNM_IMG_DATA_FORMAT_1_REVERSED:
		return 8;
	case GNM_IMG_DATA_FORMAT_GB_GR:
	case GNM_IMG_DATA_FORMAT_BG_RG:
		return 2;
	default:
		return 1;
	}
}
static inline uint32_t gnmDfGetTexelsPerElementTall(const GnmDataFormat fmt) {
	switch (fmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_BC1:
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
	case GNM_IMG_DATA_FORMAT_BC4:
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_BC7:
		return 4;
	default:
		return 1;
	}
}

static const GnmDataFormat GNM_FMT_INVALID = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_INVALID,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_CONSTANT0,
    .chany = GNM_CHAN_CONSTANT0,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT0,
};
static const GnmDataFormat GNM_FMT_R8_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_CONSTANT0,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_A8_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_CONSTANT0,
    .chany = GNM_CHAN_CONSTANT0,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_X,
};
static const GnmDataFormat GNM_FMT_R8G8B8A8_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R8G8B8A8_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R8G8B8A8_UINT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8,
    .chantype = GNM_IMG_NUM_FORMAT_UINT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_B8G8R8A8_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_Z,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_X,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_B8G8R8A8_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_Z,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_X,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R16_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_16,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_CONSTANT0,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_R16G16_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_16_16,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_R16G16B16A16_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_16_16_16_16,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R16G16B16A16_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_16_16_16_16,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R16G16B16A16_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_16_16_16_16,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R32_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_CONSTANT0,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_R32G32_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_CONSTANT0,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_R32G32B32_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_CONSTANT0,
};
static const GnmDataFormat GNM_FMT_R32G32B32_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_R32G32B32A32_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R32G32B32A32_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_R32G32B32A32_FLOAT = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_32_32_32_32,
    .chantype = GNM_IMG_NUM_FORMAT_FLOAT,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_BC1_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC1,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_BC1_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC1,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_BC3_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC3,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_BC6_SNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC6,
    .chantype = GNM_IMG_NUM_FORMAT_SNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_BC6_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC6,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_CONSTANT1,
};
static const GnmDataFormat GNM_FMT_BC7_UNORM = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC7,
    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};
static const GnmDataFormat GNM_FMT_BC7_SRGB = {
    .surfacefmt = GNM_IMG_DATA_FORMAT_BC7,
    .chantype = GNM_IMG_NUM_FORMAT_SRGB,
    .chanx = GNM_CHAN_X,
    .chany = GNM_CHAN_Y,
    .chanz = GNM_CHAN_Z,
    .chanw = GNM_CHAN_W,
};

#endif	// _GNM_DATAFORMAT_H_
