#include "gpuaddr_private.h"

GpaError gpaFindOptimalSurface(
    GpaSurfaceProperties* outprops, GpaSurfaceType surfacetype, uint32_t bpp,
    uint32_t numfrags, bool mipmapped, GnmGpuMode mingpumode
) {
	if (!outprops) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaSurfaceFlags flags = {0};
	switch (surfacetype) {
	case GPA_SURFACE_COLORDISPLAY:
		flags.display = 1;
		break;
	case GPA_SURFACE_COLOR:
		break;
	case GPA_SURFACE_DEPTHSTENCIL:
		flags.depthtarget = 1;
		flags.stenciltarget = 1;
		break;
	case GPA_SURFACE_DEPTH:
		flags.depthtarget = 1;
		break;
	case GPA_SURFACE_STENCIL:
		flags.stenciltarget = 1;
		break;
	case GPA_SURFACE_FMASK:
		flags.fmask = 1;
		break;
	case GPA_SURFACE_TEXTUREFLAT:
	case GPA_SURFACE_RWTEXTUREFLAT:
		flags.pow2pad = mipmapped;
		flags.texcompatible = mingpumode == GNM_GPU_NEO;
		break;
	case GPA_SURFACE_TEXTUREVOLUME:
	case GPA_SURFACE_RWTEXTUREVOLUME:
		flags.volume = 1;
		flags.pow2pad = mipmapped;
		flags.texcompatible = mingpumode == GNM_GPU_NEO;
		break;
	case GPA_SURFACE_TEXTURECUBEMAP:
	case GPA_SURFACE_RWTEXTURECUBEMAP:
		flags.cube = 1;
		flags.pow2pad = mipmapped;
		flags.texcompatible = mingpumode == GNM_GPU_NEO;
		break;
	default:
		return GPA_ERR_INVALID_ARGS;
	}

	/* Set the requested tiling mode. */
	GnmArrayMode arraymode = 0;
	switch (surfacetype) {
	case GPA_SURFACE_COLORDISPLAY:
	case GPA_SURFACE_COLOR:
	case GPA_SURFACE_DEPTHSTENCIL:
	case GPA_SURFACE_DEPTH:
	case GPA_SURFACE_STENCIL:
	case GPA_SURFACE_FMASK:
		arraymode = flags.prt ? GNM_ARRAY_PRT_2D_TILED_THIN1
				      : GNM_ARRAY_2D_TILED_THIN1;
		break;
	case GPA_SURFACE_TEXTUREFLAT:
	case GPA_SURFACE_RWTEXTUREFLAT:
	case GPA_SURFACE_TEXTURECUBEMAP:
	case GPA_SURFACE_RWTEXTURECUBEMAP:
		/* MSAA requires 2D tiling. */
		if (flags.prt) {
			arraymode = numfrags > 1 ? GNM_ARRAY_PRT_2D_TILED_THIN1
						 : GNM_ARRAY_PRT_TILED_THIN1;
		} else {
			arraymode = numfrags > 1 ? GNM_ARRAY_2D_TILED_THIN1
						 : GNM_ARRAY_1D_TILED_THIN1;
		}
		break;
	case GPA_SURFACE_TEXTUREVOLUME:
	case GPA_SURFACE_RWTEXTUREVOLUME:
		arraymode = flags.prt ? GNM_ARRAY_PRT_TILED_THICK
				      : GNM_ARRAY_1D_TILED_THICK;
		break;
	default:
		return GPA_ERR_INVALID_ARGS;
	}

	/* Set the micro tile type. */
	GnmMicroTileMode microtilemode = GNM_SURF_THIN_MICRO_TILING;
	if (flags.display)
		microtilemode = GNM_SURF_DISPLAY_MICRO_TILING;
	else if (flags.depthtarget || flags.stenciltarget)
		microtilemode = GNM_SURF_DEPTH_MICRO_TILING;

	/* Find the tile mode type */
	GnmTileMode tilemode = 0;
	if (microtilemode == GNM_SURF_DEPTH_MICRO_TILING) {
		const uint32_t tilesize = gpaGetMicroTileThickness(arraymode) *
					  bpp * numfrags * MICROTILE_SIZE *
					  MICROTILE_SIZE / 8;
		if (mingpumode == GNM_GPU_NEO && DRAM_ROW_SIZE < tilesize) {
			flags.texcompatible = 0;
		}
		if (flags.depthtarget && flags.texcompatible) {
			switch (tilesize) {
			case 128:
				tilemode = GNM_TM_DEPTH_2D_THIN_128;
				break;
			case 256:
				tilemode = GNM_TM_DEPTH_2D_THIN_256;
				break;
			case 512:
				tilemode = GNM_TM_DEPTH_2D_THIN_512;
				break;
			default:
				tilemode = GNM_TM_DEPTH_2D_THIN_1K;
				break;
			}
		} else {
			switch (numfrags) {
			case 1:
				tilemode = GNM_TM_DEPTH_2D_THIN_64;
				break;
			case 2:
			case 4:
				tilemode = GNM_TM_DEPTH_2D_THIN_128;
				break;
			case 8:
				tilemode = GNM_TM_DEPTH_2D_THIN_256;
				break;
			default:
				return GPA_ERR_INVALID_ARGS;
			}
		}

		switch (arraymode) {
		case GNM_ARRAY_1D_TILED_THIN1:
			tilemode = GNM_TM_DEPTH_1D_THIN;
			break;
		case GNM_ARRAY_PRT_TILED_THIN1:
			tilemode = GNM_TM_DEPTH_2D_THIN_PRT_256;
			break;
		default:
			break;
		}

		if (flags.depthtarget && !flags.stenciltarget &&
		    mingpumode == GNM_GPU_NEO &&
		    tilemode < GNM_TM_DEPTH_2D_THIN_256) {
			tilemode = GNM_TM_DEPTH_2D_THIN_256;
		}
	} else if (microtilemode == GNM_SURF_DISPLAY_MICRO_TILING) {
		if (arraymode == GNM_ARRAY_1D_TILED_THIN1) {
			tilemode = GNM_TM_DISPLAY_1D_THIN;
		} else if (arraymode == GNM_ARRAY_2D_TILED_THIN1) {
			tilemode = GNM_TM_DISPLAY_2D_THIN;
		} else if (arraymode == GNM_ARRAY_PRT_TILED_THIN1) {
			tilemode = GNM_TM_DISPLAY_THIN_PRT;
		} else if (arraymode == GNM_ARRAY_PRT_2D_TILED_THIN1) {
			tilemode = GNM_TM_DISPLAY_2D_THIN_PRT;
		} else {
			tilemode = GNM_TM_DISPLAY_1D_THIN;
		}
	} else if (microtilemode == GNM_SURF_THIN_MICRO_TILING) {
		if (arraymode == GNM_ARRAY_1D_TILED_THIN1) {
			tilemode = GNM_TM_THIN_1D_THIN;
		} else if (arraymode == GNM_ARRAY_2D_TILED_THIN1) {
			tilemode = GNM_TM_THIN_2D_THIN;
		} else if (arraymode == GNM_ARRAY_3D_TILED_THIN1) {
			tilemode = GNM_TM_THIN_3D_THIN;
		} else if (arraymode == GNM_ARRAY_PRT_TILED_THIN1) {
			tilemode = GNM_TM_THIN_THIN_PRT;
		} else if (arraymode == GNM_ARRAY_PRT_2D_TILED_THIN1) {
			tilemode = GNM_TM_THIN_2D_THIN_PRT;
		} else if (arraymode == GNM_ARRAY_PRT_3D_TILED_THIN1) {
			tilemode = GNM_TM_THIN_3D_THIN_PRT;
		} else {
			tilemode = GNM_TM_THIN_1D_THIN;
		}
	} else if (microtilemode == GNM_SURF_THICK_MICRO_TILING) {
		if (arraymode == GNM_ARRAY_1D_TILED_THICK) {
			tilemode = GNM_TM_THICK_1D_THICK;
		} else if (arraymode == GNM_ARRAY_2D_TILED_THICK) {
			tilemode = GNM_TM_THICK_2D_THICK;
		} else if (arraymode == GNM_ARRAY_3D_TILED_THICK) {
			tilemode = GNM_TM_THICK_3D_THICK;
		} else if (arraymode == GNM_ARRAY_PRT_TILED_THICK) {
			tilemode = GNM_TM_THICK_THICK_PRT;
		} else if (arraymode == GNM_ARRAY_PRT_2D_TILED_THICK) {
			tilemode = GNM_TM_THICK_2D_THICK_PRT;
		} else if (arraymode == GNM_ARRAY_PRT_3D_TILED_THICK) {
			tilemode = GNM_TM_THICK_3D_THICK_PRT;
		} else if (arraymode == GNM_ARRAY_2D_TILED_XTHICK) {
			tilemode = GNM_TM_THICK_2D_XTHICK;
		} else if (arraymode == GNM_ARRAY_3D_TILED_XTHICK) {
			tilemode = GNM_TM_THICK_3D_XTHICK;
		} else {
			tilemode = GNM_TM_THICK_1D_THICK;
		}
	} else if (microtilemode == GNM_SURF_ROTATED_MICRO_TILING) {
		return GPA_ERR_INTERNAL_ERROR;
	}

	*outprops = (GpaSurfaceProperties){
	    .tilemode = tilemode,
	    .flags = flags,
	};
	return GPA_ERR_OK;
}
