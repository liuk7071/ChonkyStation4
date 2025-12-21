#include "gpuaddr_private.h"

#include <math.h>

GnmArrayMode gpaGetArrayMode(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_THIN_1D_THIN:
		return GNM_ARRAY_1D_TILED_THIN1;
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_2D_THIN_128:
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_512:
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_THIN_2D_THIN:
		return GNM_ARRAY_2D_TILED_THIN1;
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_THIN_THIN_PRT:
		return GNM_ARRAY_PRT_TILED_THIN1;
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_THIN_2D_THIN_PRT:
		return GNM_ARRAY_PRT_2D_TILED_THIN1;
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_3D_THIN_PRT:
		return GNM_ARRAY_3D_TILED_THIN1;
	case GNM_TM_THICK_1D_THICK:
		return GNM_ARRAY_1D_TILED_THICK;
	case GNM_TM_THICK_2D_THICK:
		return GNM_ARRAY_2D_TILED_THICK;
	case GNM_TM_THICK_3D_THICK:
		return GNM_ARRAY_3D_TILED_THICK;
	case GNM_TM_THICK_THICK_PRT:
		return GNM_ARRAY_PRT_TILED_THICK;
	case GNM_TM_THICK_2D_THICK_PRT:
		return GNM_ARRAY_PRT_2D_TILED_THICK;
	case GNM_TM_THICK_3D_THICK_PRT:
		return GNM_ARRAY_PRT_3D_TILED_THICK;
	case GNM_TM_THICK_2D_XTHICK:
		return GNM_ARRAY_2D_TILED_XTHICK;
	case GNM_TM_THICK_3D_XTHICK:
		return GNM_ARRAY_3D_TILED_XTHICK;
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
		return GNM_ARRAY_LINEAR_ALIGNED;
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_ARRAY_LINEAR_GENERAL;
	default:
		abort();
	}
}

GnmMicroTileMode gpaGetMicroTileMode(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_2D_THIN_128:
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_512:
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
		return GNM_SURF_DEPTH_MICRO_TILING;
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_SURF_DISPLAY_MICRO_TILING;
	case GNM_TM_THIN_1D_THIN:
	case GNM_TM_THIN_2D_THIN:
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_THIN_PRT:
	case GNM_TM_THIN_2D_THIN_PRT:
	case GNM_TM_THIN_3D_THIN_PRT:
		return GNM_SURF_THIN_MICRO_TILING;
	case GNM_TM_THICK_1D_THICK:
	case GNM_TM_THICK_2D_THICK:
	case GNM_TM_THICK_3D_THICK:
	case GNM_TM_THICK_THICK_PRT:
	case GNM_TM_THICK_2D_THICK_PRT:
	case GNM_TM_THICK_3D_THICK_PRT:
	case GNM_TM_THICK_2D_XTHICK:
	case GNM_TM_THICK_3D_XTHICK:
		return GNM_SURF_THICK_MICRO_TILING;
	default:
		abort();
	}
}

GnmPipeConfig gpaGetPipeConfig(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_2D_THIN_128:
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_512:
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_THIN_1D_THIN:
	case GNM_TM_THIN_2D_THIN:
	case GNM_TM_THIN_2D_THIN_PRT:
	case GNM_TM_THIN_3D_THIN_PRT:
	case GNM_TM_THICK_1D_THICK:
	case GNM_TM_THICK_2D_THICK:
	case GNM_TM_THICK_2D_THICK_PRT:
	case GNM_TM_THICK_2D_XTHICK:
		return GNM_ADDR_SURF_P8_32x32_16x16;
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_THIN_PRT:
	case GNM_TM_THICK_3D_THICK:
	case GNM_TM_THICK_THICK_PRT:
	case GNM_TM_THICK_3D_THICK_PRT:
	case GNM_TM_THICK_3D_XTHICK:
		return GNM_ADDR_SURF_P8_32x32_8x16;
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_ADDR_SURF_P2;
	default:
		abort();
	}
}

GnmPipeConfig gpaGetAltPipeConfig(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_2D_THIN_128:
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_512:
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_THIN_1D_THIN:
	case GNM_TM_THIN_2D_THIN:
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_THIN_PRT:
	case GNM_TM_THIN_2D_THIN_PRT:
	case GNM_TM_THIN_3D_THIN_PRT:
	case GNM_TM_THICK_1D_THICK:
	case GNM_TM_THICK_2D_THICK:
	case GNM_TM_THICK_3D_THICK:
	case GNM_TM_THICK_THICK_PRT:
	case GNM_TM_THICK_2D_THICK_PRT:
	case GNM_TM_THICK_3D_THICK_PRT:
	case GNM_TM_THICK_2D_XTHICK:
	case GNM_TM_THICK_3D_XTHICK:
		return GNM_ADDR_SURF_P16_32x32_8x16;
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_ADDR_SURF_P2;
	default:
		abort();
	}
}

GnmSampleSplit gpaGetSampleSplit(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_2D_THIN_128:
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_512:
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_THIN_1D_THIN:
	case GNM_TM_THICK_1D_THICK:
	case GNM_TM_THICK_2D_THICK:
	case GNM_TM_THICK_3D_THICK:
	case GNM_TM_THICK_THICK_PRT:
	case GNM_TM_THICK_2D_THICK_PRT:
	case GNM_TM_THICK_3D_THICK_PRT:
	case GNM_TM_THICK_2D_XTHICK:
	case GNM_TM_THICK_3D_XTHICK:
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_ADDR_SAMPLE_SPLIT_1;
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_THIN_2D_THIN:
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_THIN_PRT:
	case GNM_TM_THIN_2D_THIN_PRT:
	case GNM_TM_THIN_3D_THIN_PRT:
		return GNM_ADDR_SAMPLE_SPLIT_2;
	default:
		abort();
	}
}

GnmTileSplit gpaGetTileSplit(GnmTileMode tilemode) {
	switch (tilemode) {
	case GNM_TM_DEPTH_2D_THIN_64:
	case GNM_TM_DEPTH_1D_THIN:
	case GNM_TM_DISPLAY_LINEAR_ALIGNED:
	case GNM_TM_DISPLAY_1D_THIN:
	case GNM_TM_DISPLAY_2D_THIN:
	case GNM_TM_DISPLAY_THIN_PRT:
	case GNM_TM_DISPLAY_2D_THIN_PRT:
	case GNM_TM_THIN_1D_THIN:
	case GNM_TM_THIN_2D_THIN:
	case GNM_TM_THIN_3D_THIN:
	case GNM_TM_THIN_THIN_PRT:
	case GNM_TM_THIN_2D_THIN_PRT:
	case GNM_TM_THIN_3D_THIN_PRT:
	case GNM_TM_THICK_1D_THICK:
	case GNM_TM_THICK_2D_THICK:
	case GNM_TM_THICK_3D_THICK:
	case GNM_TM_THICK_THICK_PRT:
	case GNM_TM_THICK_2D_THICK_PRT:
	case GNM_TM_THICK_3D_THICK_PRT:
	case GNM_TM_THICK_2D_XTHICK:
	case GNM_TM_THICK_3D_XTHICK:
	case GNM_TM_DISPLAY_LINEAR_GENERAL:
		return GNM_SURF_TILE_SPLIT_64B;
	case GNM_TM_DEPTH_2D_THIN_128:
		return GNM_SURF_TILE_SPLIT_128B;
	case GNM_TM_DEPTH_2D_THIN_256:
	case GNM_TM_DEPTH_2D_THIN_PRT_256:
		return GNM_SURF_TILE_SPLIT_256B;
	case GNM_TM_DEPTH_2D_THIN_512:
		return GNM_SURF_TILE_SPLIT_512B;
	case GNM_TM_DEPTH_2D_THIN_1K:
	case GNM_TM_DEPTH_2D_THIN_PRT_1K:
		return GNM_SURF_TILE_SPLIT_1KB;
	default:
		abort();
	}
}

GpaError gpaCalcSurfaceMacrotileMode(
    GnmMacroTileMode* outmtm, GnmTileMode tilemode, uint32_t bitsperelem,
    uint32_t numfragsperpixel
) {
	if (!outmtm) {
		return GPA_ERR_INVALID_ARGS;
	}
	if (!IsPow2(numfragsperpixel) || numfragsperpixel > 16) {
		return GPA_ERR_INVALID_ARGS;
	}
	if (bitsperelem < 1 || bitsperelem > 128) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmArrayMode arraymode = gpaGetArrayMode(tilemode);
	if (!gpaIsMacroTiled(arraymode)) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmMicroTileMode mtm = gpaGetMicroTileMode(tilemode);
	const GnmSampleSplit samplesplithw = gpaGetSampleSplit(tilemode);
	const GnmTileSplit tilesplithw = gpaGetTileSplit(tilemode);

	const uint32_t tilethickness = gpaGetMicroTileThickness(arraymode);
	const uint32_t tilebytes1x =
	    bitsperelem * MICROTILE_SIZE * MICROTILE_SIZE * tilethickness / 8;
	const uint32_t samplesplit = 1 << samplesplithw;
	const uint32_t colortilesplit = umax(256, samplesplit * tilebytes1x);
	const uint32_t tilesplit = (mtm == GNM_SURF_DEPTH_MICRO_TILING)
				       ? (64u << tilesplithw)
				       : colortilesplit;
	const uint32_t tilesplic = umin(DRAM_ROW_SIZE, tilesplit);
	const uint32_t tilebytes =
	    umin(tilesplic, numfragsperpixel * tilebytes1x);
	const uint32_t mtmidx = log2((uint32_t)(tilebytes / 64));

	*outmtm = gpaIsPrt(arraymode) ? (mtmidx + 8) : mtmidx;
	return GPA_ERR_OK;
}

GpaError gpaAdjustTileMode(
    GnmTileMode* outtilemode, GnmTileMode oldtilemode, GnmArrayMode newarraymode
) {
	if (!outtilemode) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmArrayMode oldarraymode = gpaGetArrayMode(oldtilemode);
	if (newarraymode == oldarraymode) {
		*outtilemode = oldtilemode;
		return GPA_ERR_OK;
	}

	const GnmMicroTileMode mtm = gpaGetMicroTileMode(oldtilemode);
	switch (mtm) {
	case GNM_SURF_DEPTH_MICRO_TILING:
		if (newarraymode != GNM_ARRAY_1D_TILED_THIN1) {
			return GPA_ERR_TILING_ERROR;
		}
		*outtilemode = GNM_TM_DEPTH_1D_THIN;
		return GPA_ERR_OK;
	case GNM_SURF_DISPLAY_MICRO_TILING:
		if (newarraymode == GNM_ARRAY_1D_TILED_THIN1) {
			*outtilemode = GNM_TM_DISPLAY_1D_THIN;
		} else {
			break;
		}
		return GPA_ERR_OK;
	case GNM_SURF_THICK_MICRO_TILING:
		if (newarraymode == GNM_ARRAY_3D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_3D_THICK;
		} else if (newarraymode == GNM_ARRAY_2D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_2D_THICK;
		} else if (newarraymode == GNM_ARRAY_1D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_1D_THICK;
		} else if (newarraymode == GNM_ARRAY_3D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_3D_THIN;
		} else if (newarraymode == GNM_ARRAY_PRT_3D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_3D_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_2D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_2D_THIN;
		} else if (newarraymode == GNM_ARRAY_PRT_2D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_2D_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_PRT_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_1D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_1D_THIN;
		} else {
			break;
		}
		return GPA_ERR_OK;
	case GNM_SURF_THIN_MICRO_TILING:
		if (newarraymode == GNM_ARRAY_3D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_3D_THICK;
		} else if (newarraymode == GNM_ARRAY_2D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_2D_THICK;
		} else if (newarraymode == GNM_ARRAY_1D_TILED_THICK) {
			*outtilemode = GNM_TM_THICK_1D_THICK;
		} else if (newarraymode == GNM_ARRAY_3D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_3D_THIN;
		} else if (newarraymode == GNM_ARRAY_PRT_3D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_3D_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_2D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_2D_THIN;
		} else if (newarraymode == GNM_ARRAY_PRT_2D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_2D_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_PRT_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_THIN_PRT;
		} else if (newarraymode == GNM_ARRAY_1D_TILED_THIN1) {
			*outtilemode = GNM_TM_THIN_1D_THIN;
		} else {
			break;
		}
		return GPA_ERR_OK;
	case GNM_SURF_ROTATED_MICRO_TILING:
	default:
		return GPA_ERR_INVALID_ARGS;
	}

	return GPA_ERR_UNSUPPORTED;
}

uint32_t gpaGetMicroTileThickness(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THIN1:
		return 1;
	case GNM_ARRAY_1D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return 4;
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
		return 8;
	default:
		abort();
	}
}

bool gpaIsLinear(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
		return true;
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_1D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return false;
	default:
		abort();
	}
}

bool gpaIsMicroTiled(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_1D_TILED_THICK:
		return true;
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return false;
	default:
		abort();
	}
}

bool gpaIsMacroTiled(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_1D_TILED_THICK:
		return false;
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return true;
	default:
		abort();
	}
}

static bool ismacrotiled3d(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_1D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
		return false;
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return true;
	default:
		abort();
	}
}

bool gpaIsPrt(GnmArrayMode arraymode) {
	switch (arraymode) {
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		return true;
	case GNM_ARRAY_LINEAR_GENERAL:
	case GNM_ARRAY_LINEAR_ALIGNED:
	case GNM_ARRAY_1D_TILED_THIN1:
	case GNM_ARRAY_1D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_THIN1:
	case GNM_ARRAY_2D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
		return false;
	default:
		abort();
	}
}

//
// BASE mode macrotilemode stuff
//
GnmBankWidth gpaGetBankWidth(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x8_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_BANK_WIDTH_1;
	default:
		abort();
	}
}

GnmBankHeight gpaGetBankHeight(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_BANK_HEIGHT_1;
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x2_16_DUP:
		return GNM_SURF_BANK_HEIGHT_2;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
		return GNM_SURF_BANK_HEIGHT_4;
	case GNM_MACROTILEMODE_1x8_16:
		return GNM_SURF_BANK_HEIGHT_8;
	default:
		abort();
	}
}

GnmNumBanks gpaGetNumBanks(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_2_BANK;
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_4_DUP:
		return GNM_SURF_4_BANK;
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_8_DUP:
		return GNM_SURF_8_BANK;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x8_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
		return GNM_SURF_16_BANK;
	default:
		abort();
	}
}

GnmMacroTileAspect gpaGetMacrotileAspect(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_MACRO_ASPECT_1;
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
		return GNM_SURF_MACRO_ASPECT_2;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x8_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
		return GNM_SURF_MACRO_ASPECT_4;
	default:
		abort();
	}
}

//
// NEO mode macrotilemode stuff
//
GnmBankHeight gpaGetAltBankHeight(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_BANK_HEIGHT_1;
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
		return GNM_SURF_BANK_HEIGHT_2;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x8_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
		return GNM_SURF_BANK_HEIGHT_4;
	default:
		abort();
	}
}

GnmNumBanks gpaGetAltNumBanks(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_2_BANK;
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
		return GNM_SURF_4_BANK;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x4_16_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
		return GNM_SURF_8_BANK;
	case GNM_MACROTILEMODE_1x8_16:
		return GNM_SURF_16_BANK;
	default:
		abort();
	}
}

GnmMacroTileAspect gpaGetAltMacrotileAspect(GnmMacroTileMode mtm) {
	switch (mtm) {
	case GNM_MACROTILEMODE_1x1_16:
	case GNM_MACROTILEMODE_1x1_16_DUP:
	case GNM_MACROTILEMODE_1x1_8:
	case GNM_MACROTILEMODE_1x1_4:
	case GNM_MACROTILEMODE_1x1_2:
	case GNM_MACROTILEMODE_1x1_2_DUP:
	case GNM_MACROTILEMODE_1x2_16_DUP:
	case GNM_MACROTILEMODE_1x1_16_DUP2:
	case GNM_MACROTILEMODE_1x1_8_DUP:
	case GNM_MACROTILEMODE_1x1_4_DUP:
	case GNM_MACROTILEMODE_1x1_2_DUP2:
	case GNM_MACROTILEMODE_1x1_2_DUP3:
		return GNM_SURF_MACRO_ASPECT_1;
	case GNM_MACROTILEMODE_1x4_16:
	case GNM_MACROTILEMODE_1x2_16:
	case GNM_MACROTILEMODE_1x8_16:
	case GNM_MACROTILEMODE_1x4_16_DUP:
		return GNM_SURF_MACRO_ASPECT_2;
	default:
		abort();
	}
}

uint32_t gpaGetPipeCount(GnmPipeConfig pipecfg) {
	switch (pipecfg) {
	case GNM_ADDR_SURF_P2:
		return 2;
	case GNM_ADDR_SURF_P8_32x32_8x16:
	case GNM_ADDR_SURF_P8_32x32_16x16:
		return 8;
	case GNM_ADDR_SURF_P16_32x32_8x16:
		return 16;
	default:
		abort();
	}
}

GpaError gpaGetTileInfo(
    GpaTileInfo* outinfo, GnmTileMode tilemode, uint32_t bpp, uint32_t numfrags,
    GnmGpuMode gpumode
) {
	if (!outinfo || tilemode < GNM_TM_DEPTH_2D_THIN_64 ||
	    tilemode > GNM_TM_DISPLAY_LINEAR_GENERAL) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmArrayMode arraymode = gpaGetArrayMode(tilemode);

	GnmNumBanks banks = GNM_SURF_2_BANK;
	GnmBankWidth bankw = GNM_SURF_BANK_WIDTH_1;
	GnmBankHeight bankh = GNM_SURF_BANK_HEIGHT_1;
	GnmMacroTileAspect macroaspect = GNM_SURF_MACRO_ASPECT_1;
	const GnmTileSplit tilesplit = gpaGetTileSplit(tilemode);
	const GnmPipeConfig pipeconfig = gpumode == GNM_GPU_NEO
					     ? gpaGetAltPipeConfig(tilemode)
					     : gpaGetPipeConfig(tilemode);

	if (gpaIsMacroTiled(arraymode)) {
		GnmMacroTileMode macrotilemode = GNM_MACROTILEMODE_1x1_2;
		GpaError err = gpaCalcSurfaceMacrotileMode(
		    &macrotilemode, tilemode, bpp, numfrags
		);
		if (err != GPA_ERR_OK) {
			return err;
		}

		if (gpumode == GNM_GPU_NEO) {
			banks = gpaGetAltNumBanks(macrotilemode);
			bankh = gpaGetAltBankHeight(macrotilemode);
			macroaspect = gpaGetAltMacrotileAspect(macrotilemode);
		} else {
			banks = gpaGetNumBanks(macrotilemode);
			bankh = gpaGetBankHeight(macrotilemode);
			macroaspect = gpaGetMacrotileAspect(macrotilemode);
		}

		bankw = gpaGetBankWidth(macrotilemode);
	}

	*outinfo = (GpaTileInfo){
	    .arraymode = arraymode,
	    .banks = banks,
	    .bankwidth = bankw,
	    .bankheight = bankh,
	    .macroaspectratio = macroaspect,
	    .tilesplit = tilesplit,
	    .pipeconfig = pipeconfig,
	};
	return GPA_ERR_OK;
}

static uint32_t GetBankPipeSwizzle(
    uint32_t bankSwizzle, uint32_t pipeSwizzle, uint64_t baseAddr,
    const GpaTileInfo* tileinfo
) {
	const uint32_t numPipes = gpaGetPipeCount(tileinfo->pipeconfig);
	const uint32_t pipeBits = QLog2(numPipes);
	const uint32_t bankInterleaveBits = QLog2(BANK_INTERLEAVE);
	const uint32_t tileSwizzle =
	    pipeSwizzle + ((bankSwizzle << bankInterleaveBits) << pipeBits);

	baseAddr ^= tileSwizzle * PIPE_INTERLEAVE_BYTES;
	baseAddr >>= 8;

	return (uint32_t)baseAddr;
}

GpaError gpaComputeBaseSwizzle(
    uint32_t* outswizzle, GnmTileMode tilemode, uint32_t surfindex,
    uint32_t bpp, uint32_t numfrags, GnmGpuMode gpumode
) {
	if (!outswizzle) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaTileInfo tileinfo = {0};
	GpaError err =
	    gpaGetTileInfo(&tileinfo, tilemode, bpp, numfrags, gpumode);
	if (err != GPA_ERR_OK) {
		return err;
	}

	if (!gpaIsMacroTiled(tileinfo.arraymode)) {
		*outswizzle = 0;
		return GPA_ERR_OK;
	}

	/// This is a legacy misreading of h/w doc, use it as it doesn't hurt.
	static const uint8_t bankRotationArray[4][16] = {
	    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	     0},  // ADDR_SURF_2_BANK
	    {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	     0},  // ADDR_SURF_4_BANK
	    {0, 3, 6, 1, 4, 7, 2, 5, 0, 0, 0, 0, 0, 0, 0,
	     0},  // ADDR_SURF_8_BANK
	    {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2,
	     9},  // ADDR_SURF_16_BANK
	};

	const uint32_t numBanks = 2 << tileinfo.banks;
	const uint32_t numPipes = gpaGetPipeCount(tileinfo.pipeconfig);

	const uint32_t bankSwizzle =
	    bankRotationArray[tileinfo.banks][surfindex & (numBanks - 1)];
	uint32_t pipeswizzle = 0;
	if (ismacrotiled3d(tileinfo.arraymode)) {
		pipeswizzle = surfindex & (numPipes - 1);
	}

	*outswizzle =
	    GetBankPipeSwizzle(bankSwizzle, pipeswizzle, 0, &tileinfo);
	return GPA_ERR_OK;
}
