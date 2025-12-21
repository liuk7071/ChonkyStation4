#include "dataformat.h"

#include <stdlib.h>

GnmDataFormat gnmDfInitFromFmask(uint32_t numsamples, uint32_t numfrags) {
	GnmDataFormat res = {
	    .surfacefmt = GNM_IMG_DATA_FORMAT_INVALID,
	    .chantype = GNM_IMG_NUM_FORMAT_UNORM,
	    .chanx = GNM_CHAN_X,
	    .chany = GNM_CHAN_X,
	    .chanz = GNM_CHAN_CONSTANT0,
	    .chanw = GNM_CHAN_CONSTANT1,
	};

	switch (numsamples) {
	case 1:
		// invalid
		break;
	case 2:
		switch (numfrags) {
		case 1:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S2_F1;
			break;
		case 2:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S2_F2;
			break;
		case 4:
		case 8:
			// invalid
			break;
		default:
			abort();
		}
		break;
	case 4:
		switch (numfrags) {
		case 1:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S4_F1;
			break;
		case 2:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S4_F2;
			break;
		case 4:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S4_F4;
			break;
		case 8:
			// invalid
			break;
		default:
			abort();
		}
		break;
	case 8:
		switch (numfrags) {
		case 1:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK8_S8_F1;
			break;
		case 2:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK16_S8_F2;
			break;
		case 4:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK32_S8_F4;
			break;
		case 8:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK32_S8_F8;
			break;
		default:
			abort();
		}
		break;
	case 16:
		switch (numfrags) {
		case 1:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK16_S16_F1;
			break;
		case 2:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK32_S16_F2;
			break;
		case 4:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK64_S16_F4;
			break;
		case 8:
			res.surfacefmt = GNM_IMG_DATA_FORMAT_FMASK64_S16_F8;
			break;
		default:
			abort();
		}
		break;
	default:
		// TODO: error out instead
		abort();
	}

	if (numsamples == 16) {
		res.chany = GNM_CHAN_Y;
		res.chanz = GNM_CHAN_CONSTANT1;
	}

	return res;
}

GnmDataFormat gnmDfInitFromZ(GnmZFormat zfmt) {
	GnmImageFormat surfmt = GNM_IMG_DATA_FORMAT_INVALID;
	GnmImgNumFormat chantype = GNM_IMG_NUM_FORMAT_UNORM;
	switch (zfmt) {
	case GNM_Z_INVALID:
	default:
		// surfmt = GNM_IMG_DATA_FORMAT_INVALID;
		//  chantype = GNM_IMG_NUM_FORMAT_UNORM;
		break;
	case GNM_Z_16:
		surfmt = GNM_IMG_DATA_FORMAT_16;
		// chantype = GNM_IMG_NUM_FORMAT_UNORM;
		break;
	case GNM_Z_32_FLOAT:
		surfmt = GNM_IMG_DATA_FORMAT_32;
		chantype = GNM_IMG_NUM_FORMAT_FLOAT;
		break;
	}

	GnmDataFormat res = {
	    .surfacefmt = surfmt,
	    .chantype = chantype,
	    .chanx = GNM_CHAN_X,
	    .chany = GNM_CHAN_CONSTANT0,
	    .chanz = GNM_CHAN_CONSTANT0,
	    .chanw = GNM_CHAN_CONSTANT1,
	};
	return res;
}

uint32_t gnmDfGetNumComponents(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_INVALID:
		return 0;
	case GNM_IMG_DATA_FORMAT_8:
	case GNM_IMG_DATA_FORMAT_16:
	case GNM_IMG_DATA_FORMAT_32:
	case GNM_IMG_DATA_FORMAT_BC4:
	case GNM_IMG_DATA_FORMAT_1:
	case GNM_IMG_DATA_FORMAT_1_REVERSED:
		return 1;
	case GNM_IMG_DATA_FORMAT_8_8:
	case GNM_IMG_DATA_FORMAT_16_16:
	case GNM_IMG_DATA_FORMAT_32_32:
	case GNM_IMG_DATA_FORMAT_8_24:
	case GNM_IMG_DATA_FORMAT_24_8:
	case GNM_IMG_DATA_FORMAT_X24_8_32:
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_FMASK8_S2_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S8_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S2_F2:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F2:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F4:
	case GNM_IMG_DATA_FORMAT_FMASK16_S16_F1:
	case GNM_IMG_DATA_FORMAT_FMASK16_S8_F2:
	case GNM_IMG_DATA_FORMAT_FMASK32_S16_F2:
	case GNM_IMG_DATA_FORMAT_FMASK32_S8_F4:
	case GNM_IMG_DATA_FORMAT_FMASK32_S8_F8:
	case GNM_IMG_DATA_FORMAT_FMASK64_S16_F4:
	case GNM_IMG_DATA_FORMAT_FMASK64_S16_F8:
	case GNM_IMG_DATA_FORMAT_4_4:
		return 2;
	case GNM_IMG_DATA_FORMAT_10_11_11:
	case GNM_IMG_DATA_FORMAT_11_11_10:
	case GNM_IMG_DATA_FORMAT_32_32_32:
	case GNM_IMG_DATA_FORMAT_5_6_5:
	case GNM_IMG_DATA_FORMAT_GB_GR:
	case GNM_IMG_DATA_FORMAT_BG_RG:
	case GNM_IMG_DATA_FORMAT_5_9_9_9:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_6_5_5:
		return 3;
	case GNM_IMG_DATA_FORMAT_10_10_10_2:
	case GNM_IMG_DATA_FORMAT_2_10_10_10:
	case GNM_IMG_DATA_FORMAT_8_8_8_8:
	case GNM_IMG_DATA_FORMAT_16_16_16_16:
	case GNM_IMG_DATA_FORMAT_32_32_32_32:
	case GNM_IMG_DATA_FORMAT_1_5_5_5:
	case GNM_IMG_DATA_FORMAT_5_5_5_1:
	case GNM_IMG_DATA_FORMAT_4_4_4_4:
	case GNM_IMG_DATA_FORMAT_BC1:
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
	case GNM_IMG_DATA_FORMAT_BC7:
		return 4;
	default:
		abort();
	}
}

uint32_t gnmDfGetBitsPerElement(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_INVALID:
		return 0;
	case GNM_IMG_DATA_FORMAT_8:
		return 8;
	case GNM_IMG_DATA_FORMAT_16:
	case GNM_IMG_DATA_FORMAT_8_8:
		return 16;
	case GNM_IMG_DATA_FORMAT_32:
	case GNM_IMG_DATA_FORMAT_16_16:
	case GNM_IMG_DATA_FORMAT_10_11_11:
	case GNM_IMG_DATA_FORMAT_11_11_10:
	case GNM_IMG_DATA_FORMAT_10_10_10_2:
	case GNM_IMG_DATA_FORMAT_2_10_10_10:
	case GNM_IMG_DATA_FORMAT_8_8_8_8:
		return 32;
	case GNM_IMG_DATA_FORMAT_32_32:
	case GNM_IMG_DATA_FORMAT_16_16_16_16:
		return 64;
	case GNM_IMG_DATA_FORMAT_32_32_32:
		return 96;
	case GNM_IMG_DATA_FORMAT_32_32_32_32:
		return 128;
	case GNM_IMG_DATA_FORMAT_5_6_5:
	case GNM_IMG_DATA_FORMAT_1_5_5_5:
	case GNM_IMG_DATA_FORMAT_5_5_5_1:
	case GNM_IMG_DATA_FORMAT_4_4_4_4:
		return 16;
	case GNM_IMG_DATA_FORMAT_8_24:
	case GNM_IMG_DATA_FORMAT_24_8:
		return 32;
	case GNM_IMG_DATA_FORMAT_X24_8_32:
		return 64;
	case GNM_IMG_DATA_FORMAT_GB_GR:
	case GNM_IMG_DATA_FORMAT_BG_RG:
		return 16;
	case GNM_IMG_DATA_FORMAT_5_9_9_9:
		return 32;
	case GNM_IMG_DATA_FORMAT_BC1:
		return 4;
	case GNM_IMG_DATA_FORMAT_BC2:
	case GNM_IMG_DATA_FORMAT_BC3:
		return 8;
	case GNM_IMG_DATA_FORMAT_BC4:
		return 4;
	case GNM_IMG_DATA_FORMAT_BC5:
	case GNM_IMG_DATA_FORMAT_BC6:
	case GNM_IMG_DATA_FORMAT_BC7:
		return 8;
	case GNM_IMG_DATA_FORMAT_FMASK8_S2_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S8_F1:
	case GNM_IMG_DATA_FORMAT_FMASK8_S2_F2:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F2:
	case GNM_IMG_DATA_FORMAT_FMASK8_S4_F4:
		return 8;
	case GNM_IMG_DATA_FORMAT_FMASK16_S16_F1:
	case GNM_IMG_DATA_FORMAT_FMASK16_S8_F2:
		return 16;
	case GNM_IMG_DATA_FORMAT_FMASK32_S16_F2:
	case GNM_IMG_DATA_FORMAT_FMASK32_S8_F4:
	case GNM_IMG_DATA_FORMAT_FMASK32_S8_F8:
		return 32;
	case GNM_IMG_DATA_FORMAT_FMASK64_S16_F4:
	case GNM_IMG_DATA_FORMAT_FMASK64_S16_F8:
		return 64;
	case GNM_IMG_DATA_FORMAT_4_4:
		return 8;
	case GNM_IMG_DATA_FORMAT_6_5_5:
		return 16;
	case GNM_IMG_DATA_FORMAT_1:
	case GNM_IMG_DATA_FORMAT_1_REVERSED:
		return 1;
	default:
		abort();
	}
}

bool gnmDfGetRtChannelType(const GnmDataFormat datafmt, GnmSurfaceNumber* out) {
	switch (datafmt.chantype) {
	case GNM_IMG_NUM_FORMAT_UNORM:
		*out = GNM_NUMBER_UNORM;
		break;
	case GNM_IMG_NUM_FORMAT_SNORM:
		*out = GNM_NUMBER_SNORM;
		break;
		*out = GNM_NUMBER_UINT;
	case GNM_IMG_NUM_FORMAT_UINT:
		break;
	case GNM_IMG_NUM_FORMAT_SINT:
		*out = GNM_NUMBER_SINT;
		break;
	case GNM_IMG_NUM_FORMAT_FLOAT:
		*out = GNM_NUMBER_FLOAT;
		break;
	case GNM_IMG_NUM_FORMAT_SRGB:
		*out = GNM_NUMBER_SRGB;
		break;
	default:
		return false;
	}
	return true;
}

bool gnmDfGetRtChannelOrder(const GnmDataFormat datafmt, GnmSurfaceSwap* out) {
	const uint32_t numcomps = gnmDfGetNumComponents(datafmt);
	const GnmChannel cx = datafmt.chanx;
	const GnmChannel cy = datafmt.chany;
	const GnmChannel cz = datafmt.chanz;
	const GnmChannel cw = datafmt.chanw;

	if (numcomps == 1) {
		if (cx == GNM_CHAN_X) {
			*out = GNM_SWAP_STD;
			return true;
		} else if (cy == GNM_CHAN_X) {
			*out = GNM_SWAP_ALT;
			return true;
		} else if (cz == GNM_CHAN_X) {
			*out = GNM_SWAP_STD_REV;
			return true;
		} else if (cw == GNM_CHAN_X) {
			*out = GNM_SWAP_ALT_REV;
			return true;
		}
	} else if (numcomps == 2) {
		if (cx == GNM_CHAN_X && cy == GNM_CHAN_Y) {
			*out = GNM_SWAP_STD;
			return true;
		} else if (cx == GNM_CHAN_X && cw == GNM_CHAN_Y) {
			*out = GNM_SWAP_ALT;
			return true;
		} else if (cx == GNM_CHAN_Y && cy == GNM_CHAN_X) {
			*out = GNM_SWAP_STD_REV;
			return true;
		} else if (cx == GNM_CHAN_Y && cw == GNM_CHAN_X) {
			*out = GNM_SWAP_STD_REV;
			return true;
		}
	} else if (numcomps == 3) {
		if (cx == GNM_CHAN_X && cy == GNM_CHAN_Y && cz == GNM_CHAN_Z) {
			*out = GNM_SWAP_STD;
			return true;
		} else if (cx == GNM_CHAN_X && cy == GNM_CHAN_Y && cw == GNM_CHAN_Z) {
			*out = GNM_SWAP_ALT;
			return true;
		} else if (cx == GNM_CHAN_Z && cy == GNM_CHAN_Y && cz == GNM_CHAN_X) {
			*out = GNM_SWAP_STD_REV;
			return true;
		} else if (cx == GNM_CHAN_Z && cy == GNM_CHAN_Y && cw == GNM_CHAN_X) {
			*out = GNM_SWAP_ALT_REV;
			return true;
		}
	} else if (numcomps == 4) {
		if (cx == GNM_CHAN_X && cy == GNM_CHAN_Y && cz == GNM_CHAN_Z &&
		    cw == GNM_CHAN_W) {
			*out = GNM_SWAP_STD;
			return true;
		} else if (cx == GNM_CHAN_Z && cy == GNM_CHAN_Y && cz == GNM_CHAN_X && cw == GNM_CHAN_W) {
			*out = GNM_SWAP_ALT;
			return true;
		} else if (cx == GNM_CHAN_W && cy == GNM_CHAN_Z && cz == GNM_CHAN_Y && cw == GNM_CHAN_X) {
			*out = GNM_SWAP_STD_REV;
			return true;
		} else if (cx == GNM_CHAN_Y && cy == GNM_CHAN_Z && cz == GNM_CHAN_W && cw == GNM_CHAN_X) {
			*out = GNM_SWAP_ALT_REV;
			return true;
		}
	}

	return false;
}

GnmZFormat gnmDfGetZFormat(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_16:
		return GNM_Z_16;
	case GNM_IMG_DATA_FORMAT_24_8:
		return GNM_Z_24;
	case GNM_IMG_DATA_FORMAT_32:
		return GNM_Z_32_FLOAT;
	default:
		return GNM_Z_INVALID;
	}
}

GnmStencilFormat gnmDfGetStencilFormat(const GnmDataFormat datafmt) {
	switch (datafmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_8:
		return GNM_STENCIL_8;
	default:
		return GNM_STENCIL_INVALID;
	}
}
