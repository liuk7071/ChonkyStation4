#include "texture.h"

#include "../gpuaddr.h"
#include "platform.h"

#include "../gpuaddr_private.h"

static inline GpaError tex_calcswizzlemask(
    uint32_t* outmask, const GnmTexture* tex
) {
	const GnmGpuMode gpumode = gnmGpuMode();
	const GnmTileMode tm = tex->tilingindex;
	const GnmDataFormat fmt = gnmTexGetFormat(tex);
	const uint32_t bitsperelem = gnmDfGetBitsPerElement(fmt);
	const uint8_t numfrags = gnmTexGetNumFragments(tex);

	return gpaComputeBaseSwizzle(
	    outmask, tm, 0, bitsperelem, numfrags, gpumode
	);
}

GnmError gnmCreateTexture(GnmTexture* tex, const GnmTextureCreateInfo* ci) {
	if (!tex) {
		printf("CreateTexture: tex can't be null");
	}
	if (!ci) {
		printf(
		    "CreateTexture: createinfo can't be null"
		);
	}

	if (!ci->nummiplevels) {
		printf(
		    "nummiplevels must be greater than 0"
		);
		return GNM_ERROR_INVALID_ARGS;
	}

	uint32_t largestdim = ci->width;
	if (largestdim < ci->height) {
		largestdim = ci->height;
	}
	if (largestdim < ci->depth) {
		largestdim = ci->depth;
	}

	if (ci->format.chantype != GNM_IMG_NUM_FORMAT_FLOAT &&
	    (ci->format.surfacefmt == GNM_IMG_DATA_FORMAT_10_11_11 ||
	     ci->format.surfacefmt == GNM_IMG_DATA_FORMAT_11_11_10)) {
		printf(
		    "format 0x%x cannot be used as a texture",
		    ci->format.asuint
		);
		return GNM_ERROR_INVALID_ARGS;
	}

	tex->pow2pad = ci->nummiplevels > 1;
	tex->mtype2 = 1;
	tex->type = ci->texturetype;

	switch (ci->texturetype) {
	case GNM_TEXTURE_1D:
	case GNM_TEXTURE_1D_ARRAY:
		if (ci->height != 1) {
			printf(
			    "height must be 1 if texturetype is 1D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		if (ci->depth != 1) {
			printf(
			    "depth must be 1 if texturetype is 1D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		if (ci->numfragments != 1) {
			printf(
			    "numfragments must be 1 if texturetype is 1D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		break;
	case GNM_TEXTURE_2D:
	case GNM_TEXTURE_2D_ARRAY:
		if (ci->depth != 1) {
			printf(
			    "depth must be 1 if texturetype is 2D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		if (ci->numfragments > 1) {
			printf(
			    "if numfragments is larger than 1, texturetype "
			    "must be 2D MSAA"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		break;
	case GNM_TEXTURE_3D:
		if (ci->numslices != 1) {
			printf(
			    "numslices must be 1 if texturetype is 2D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		if (ci->numfragments != 1) {
			printf(
			    "numfragments must be 1 if texturetype is 2D"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		break;
	case GNM_TEXTURE_2D_MSAA:
	case GNM_TEXTURE_2D_ARRAY_MSAA:
		if (ci->numfragments <= 1) {
			printf(
			    "numfragments must be larger than "
			    "1 if texturetype is 2D MSAA"
			);
			return GNM_ERROR_INVALID_ARGS;
		}
		break;
	case GNM_TEXTURE_CUBEMAP:
		// TODO?
		break;
	default:
		printf(
		    "Unknown texturetype %u", ci->texturetype
		);
		return GNM_ERROR_INTERNAL_FAILURE;
	}

	const uint32_t totalbitsperelem =
	    gnmDfGetTotalBitsPerElement(ci->format);
	const uint32_t texelsperelem = gnmDfGetTexelsPerElement(ci->format);

	const GpaTilingParams tp = {
	    .tilemode = ci->tilemodehint,
	    .mingpumode = ci->mingpumode,

	    .linearwidth = ci->width,
	    .linearheight = ci->height,
	    .lineardepth = ci->depth,
	    .numfragsperpixel = 1 << ci->numfragments,
	    .basetiledpitch = ci->pitch,

	    .miplevel = 0,
	    .arrayslice = 0,
	    .surfaceflags =
		{
		    .cube = ci->texturetype == GNM_TEXTURE_CUBEMAP,
		    .volume = ci->texturetype == GNM_TEXTURE_3D,
		    .pow2pad = ci->nummiplevels > 1,
		    .texcompatible = ci->mingpumode == GNM_GPU_NEO,
		},
	    .bitsperfrag = totalbitsperelem / texelsperelem,
	    .isblockcompressed = texelsperelem > 1,
	};

	GpaSurfaceInfo surfinfo = {0};
	GpaError err = gpaComputeSurfaceInfo(&surfinfo, &tp);
	if (err != GPA_ERR_OK) {
		printf(
            "Failed to compute surface info with %s",
		    gpaStrError(err)
		);
		return GNM_ERROR_INTERNAL_FAILURE;
	}

	gnmTexSetWidth(tex, ci->width);
	gnmTexSetHeight(tex, ci->height);
	gnmTexSetPitch(tex, surfinfo.pitch);

	gnmTexSetFormat(tex, ci->format);

	const uint32_t realdepth =
	    ci->texturetype == GNM_TEXTURE_3D ? ci->depth : ci->numslices;
	gnmTexSetDepth(tex, realdepth);

	// setArrayView
	const uint32_t numslices = ci->texturetype == GNM_TEXTURE_CUBEMAP
				       ? ci->numslices * 6
				       : ci->numslices;
	tex->basearray = 0;
	tex->lastarray = numslices;

	// setMipLevelRange
	tex->baselevel = 0;
	tex->lastlevel = ci->nummiplevels - 1;
	// setMinLodClamp
	// setNumFragments
	tex->minlod = 0;
	// NOTE: this is only valid for 2D (Array) MSAA textures
	if (ci->numfragments > 1) {
		tex->lastlevel = ci->numfragments >> 1;
	}
	// setTileMode
	tex->tilingindex = surfinfo.tilemode;
	// setUseAltTileMode
	tex->alttilemode = ci->mingpumode == GNM_GPU_NEO;
	// setSamplerModulationFactor
	tex->perfmod = GNM_TEX_PERFMOD_MAX;

	return GNM_ERROR_OK;
}

GnmError gnmTexCalcByteSize(
    uint64_t* outsize, uint32_t* outalignment, const GnmTexture* tex
) {
	if (!tex) {
		return GNM_ERROR_INVALID_ARGS;
	}

	const GpaTextureInfo texinfo = {
	    .type = tex->type,
	    .fmt = gnmTexGetFormat(tex),

	    .width = gnmTexGetWidth(tex),
	    .height = gnmTexGetHeight(tex),
	    .depth = gnmTexGetDepth(tex),
	    .pitch = gnmTexGetPitch(tex),

	    .numfrags = gnmTexGetNumFragments(tex),
	    .nummips = gnmTexGetNumMips(tex),
	    .numslices = gnmTexGetNumArraySlices(tex),

	    .tm = tex->tilingindex,
	    .mingpumode = tex->alttilemode ? GNM_GPU_NEO : GNM_GPU_BASE,

	    .pow2pad = tex->pow2pad,
	};

	GpaTilingParams tp = {0};
	GpaError err = gpaTpInit(&tp, &texinfo, 0, 0);
	if (err != GPA_ERR_OK) {
		return GNM_ERROR_INVALID_STATE;
	}

	GpaSurfaceInfo surfinfo = {0};
	for (uint32_t i = 0; i < texinfo.nummips; i += 1) {
		tp.linearwidth = umax(texinfo.width >> i, 1);
		tp.linearheight = umax(texinfo.height >> i, 1);
		tp.lineardepth = umax(texinfo.depth >> i, 1);
		tp.miplevel = i;

		err = gpaComputeSurfaceInfo(&surfinfo, &tp);
		if (err != GPA_ERR_OK) {
			return GNM_ERROR_INTERNAL_FAILURE;
		}

		if (outsize) {
			*outsize += texinfo.numslices * surfinfo.surfacesize;
		}
		if (outalignment) {
			*outalignment = umax(*outalignment, surfinfo.basealign);
		}

		if (tp.linearwidth == 1 && tp.linearheight == 1 &&
		    tp.lineardepth == 1) {
			break;
		}
	}

	return GNM_ERROR_OK;
}

uint32_t gnmTexGetNumArraySlices(const GnmTexture* tex) {
	uint32_t numarrayslices = gnmTexGetTotalArraySlices(tex);
	if (tex->type == GNM_TEXTURE_CUBEMAP) {
		numarrayslices *= 6;
	} else if (tex->type == GNM_TEXTURE_3D) {
		numarrayslices = 1;
	}
	if (tex->pow2pad) {
		numarrayslices = NextPow2(numarrayslices);
	}
	return numarrayslices;
}

void gnmTexSetBaseAddress(GnmTexture* tex, void* baseaddr) {
	if (((uintptr_t)baseaddr & 0xff) != 0) {
		printf(
		    "gnmTex: base address must be 256 byte aligned"
		);
		return;
	}

	uintptr_t addr = (uintptr_t)baseaddr >> 8;

	uint32_t swizzlemask = 0;
	GpaError err = tex_calcswizzlemask(&swizzlemask, tex);
	if (err == GPA_ERR_OK) {
		addr = (addr & ~swizzlemask) | (tex->baseaddress & swizzlemask);
	} else {
		printf(
		    "Texture: failed to calculate swizzle mask with %s",
		    gpaStrError(err)
		);
	}

	tex->baseaddress = addr;
}
