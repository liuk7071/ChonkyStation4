#include "gpuaddr_private.h"

#include <string.h>

GpaError gpaTpInit(
    GpaTilingParams* tp, const GpaTextureInfo* tex, uint32_t miplevel,
    uint32_t arrayslice
) {
	if (!tp || !tex) {
		return GPA_ERR_INVALID_ARGS;
	}
	if (miplevel > tex->nummips) {
		return GPA_ERR_INVALID_ARGS;
	}

	const bool iscubemap = tex->type == GNM_TEXTURE_CUBEMAP;
	const bool isvolume = tex->type == GNM_TEXTURE_3D;
	const GnmDataFormat fmt = tex->fmt;
	const GnmMicroTileMode mtm = gpaGetMicroTileMode(tex->tm);
	const uint32_t totalbitsperelem = gnmDfGetTotalBitsPerElement(fmt);
	const uint32_t texelsperelem = gnmDfGetTexelsPerElement(fmt);

	uint32_t numarrayslices = tex->numslices;
	if (tex->type == GNM_TEXTURE_CUBEMAP) {
		numarrayslices *= 6;
	} else if (tex->type == GNM_TEXTURE_3D) {
		numarrayslices = 1;
	}
	if (tex->pow2pad) {
		numarrayslices = NextPow2(numarrayslices);
	}

	if (arrayslice >= numarrayslices) {
		return GPA_ERR_INVALID_ARGS;
	}

	tp->tilemode = tex->tm;
	tp->mingpumode = tex->mingpumode;

	tp->linearwidth = umax(tex->width >> miplevel, 1);
	tp->linearheight = umax(tex->height >> miplevel, 1);
	tp->lineardepth = umax(tex->depth >> miplevel, 1);
	tp->numfragsperpixel = tex->numfrags;
	tp->basetiledpitch = tex->pitch;

	tp->miplevel = miplevel;
	tp->arrayslice = arrayslice;

	if (!isvolume && mtm == GNM_SURF_DEPTH_MICRO_TILING) {
		if (gnmDfGetZFormat(fmt) != GNM_Z_INVALID) {
			tp->surfaceflags.depthtarget = 1;
		}
		if (gnmDfGetStencilFormat(fmt) != GNM_STENCIL_INVALID) {
			tp->surfaceflags.stenciltarget = 1;
		}
	}
	tp->surfaceflags.cube = iscubemap;
	tp->surfaceflags.volume = isvolume;
	tp->surfaceflags.pow2pad = tex->pow2pad;
	if (tex->mingpumode == GNM_GPU_NEO) {
		tp->surfaceflags.texcompatible = 1;
	}

	tp->bitsperfrag = totalbitsperelem / texelsperelem;
	tp->isblockcompressed = texelsperelem > 1;

	GpaSurfaceInfo surfinfo = {0};
	GpaError err = gpaComputeSurfaceInfo(&surfinfo, tp);
	if (err != GPA_ERR_OK) {
		return err;
	}
	err = gpaAdjustTileMode(
	    &tp->tilemode, tp->tilemode, surfinfo.tileinfo.arraymode
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	return GPA_ERR_OK;
}

static GpaError initregioninfo(
    GpaSurfaceRegion* region, uint32_t* elemwidth, uint32_t* elemheight,
    const GpaTilingParams* tp
) {
	region->right = tp->linearwidth;
	region->bottom = tp->linearheight;
	region->back = tp->lineardepth;
	*elemwidth = tp->linearwidth;
	*elemheight = tp->linearheight;

	if (tp->isblockcompressed) {
		switch (tp->bitsperfrag) {
		case 1:
			region->left = (region->left + 7) / 8;
			region->right = (region->right + 7) / 8;
			*elemwidth = (*elemwidth + 7) / 8;
			break;
		case 4:
		case 8:
			region->left = (region->left + 3) / 4;
			region->top = (region->top + 3) / 4;
			region->right = (region->right + 3) / 4;
			region->bottom = (region->bottom + 3) / 4;
			*elemwidth = (*elemwidth + 3) / 4;
			*elemheight = (*elemheight + 3) / 4;
			break;
		case 16:
			return GPA_ERR_UNSUPPORTED;
		default:
			return GPA_ERR_INVALID_ARGS;
		}
	}

	return GPA_ERR_OK;
}

GpaError gpaTileSurface(
    void* outbuf, size_t outlen, const void* inbuf, size_t inlen,
    const GpaTilingParams* srctp, const GpaTilingParams* dst_tp
) {
	if (!outbuf || !outlen || !inbuf || !inlen || !srctp || !dst_tp) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaSurfaceRegion region = {0};
	uint32_t elemwidth = 0;
	uint32_t elemheight = 0;
	GpaError err = initregioninfo(&region, &elemwidth, &elemheight, srctp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	return gpaTileSurfaceRegion(
	    outbuf, outlen, inbuf, inlen, srctp, dst_tp, &region
	);
}

static inline bool regionhastexels(const GpaSurfaceRegion* region) {
	const uint32_t width = region->right - region->left;
	const uint32_t height = region->bottom - region->top;
	const uint32_t depth = region->back - region->top;
	return width > 0 && height > 0 && depth > 0;
}

GpaError gpaTileSurfaceRegion(
    void* outbuf, size_t outlen, const void* inbuf, size_t inlen,
    const GpaTilingParams* srctp, const GpaTilingParams* dst_tp,
    const GpaSurfaceRegion* region
) {
	if (!outbuf || !outlen || !inbuf || !inlen || !srctp || !dst_tp ||
	    !region) {
		return GPA_ERR_INVALID_ARGS;
	}

	if (!regionhastexels(region)) {
		// nothing to convert
		return GPA_ERR_OK;
	}

	GpaSurfaceContext src_ctx = {0};
	GpaError err = gpaInitSurfaceContext(&src_ctx, inlen, srctp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	GpaSurfaceContext dstctx = {0};
	err = gpaInitSurfaceContext(&dstctx, outlen, dst_tp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	if (src_ctx.bitsperelement != dstctx.bitsperelement ||
	    src_ctx.numfragsperpixel != dstctx.numfragsperpixel) {
		return GPA_ERR_UNSUPPORTED;
	}

	const uint32_t elembytesize = src_ctx.bitsperelement / 8;
	const uint32_t lz = region->back;
	const uint32_t ly = region->bottom;
	const uint32_t lx = region->right;
	const uint32_t lf = src_ctx.numfragsperpixel;
	for (uint32_t z = region->front; z < lz; z += 1) {
		for (uint32_t y = region->top; y < ly; y += 1) {
			for (uint32_t x = region->left; x < lx; x += 1) {
				for (uint32_t f = 0; f < lf; f += 1) {
					uint64_t srcoff = 0;
					GpaError gerr = gpaComputeSurfaceCoord(
					    &srcoff, NULL, &src_ctx, x, y, z, f
					);
					if (gerr != GPA_ERR_OK) {
						return gerr;
					}

					uint64_t dstoff = 0;
					gerr = gpaComputeSurfaceCoord(
					    &dstoff, NULL, &dstctx, x, y, z, f
					);
					if (gerr != GPA_ERR_OK) {
						return gerr;
					}

					if (srcoff + elembytesize > inlen ||
					    dstoff + elembytesize > outlen) {
						return GPA_ERR_OVERFLOW;
					}

					memcpy(
					    (uint8_t*)outbuf + dstoff,
					    (const uint8_t*)inbuf + srcoff,
					    elembytesize
					);
				}
			}
		}
	}

	return GPA_ERR_OK;
}

GpaError gpaTileTextureIndexed(
    const void* inbuf, size_t inlen, void* outbuf, size_t outlen,
    const GpaTextureInfo* texinfo, GnmTileMode newtiling, uint32_t mip,
    uint32_t slice
) {
	if (!inbuf || !inlen || !outbuf || !outlen || !texinfo) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaTilingParams srctp = {0};
	GpaError res = gpaTpInit(&srctp, texinfo, mip, slice);
	if (res != GPA_ERR_OK) {
		return res;
	}

	GpaTilingParams dst_tp = {0};
	GpaTextureInfo dstinfo = *texinfo;
	dstinfo.tm = newtiling;
	res = gpaTpInit(&dst_tp, &dstinfo, mip, slice);
	if (res != GPA_ERR_OK) {
		return res;
	}

	uint64_t srcsurfoff = 0;
	uint64_t srcsurflen = 0;
	res = gpaComputeSurfaceSizeOffset(
	    &srcsurflen, &srcsurfoff, texinfo, mip, slice
	);
	if (res != GPA_ERR_OK) {
		return res;
	}

	uint64_t dstsurfoff = 0;
	uint64_t dstsurflen = 0;
	res = gpaComputeSurfaceSizeOffset(
	    &dstsurflen, &dstsurfoff, &dstinfo, mip, slice
	);
	if (res != GPA_ERR_OK) {
		return res;
	}

	if (srcsurfoff + srcsurflen > inlen ||
	    dstsurfoff + dstsurflen > outlen) {
		return GPA_ERR_OVERFLOW;
	}

	res = gpaTileSurface(
	    (uint8_t*)outbuf + dstsurfoff, dstsurflen,
	    (const uint8_t*)inbuf + srcsurfoff, srcsurflen, &srctp, &dst_tp
	);
	if (res != GPA_ERR_OK) {
		return res;
	}

	return GPA_ERR_OK;
}

GpaError gpaTileTextureAll(
    const void* inbuf, size_t inlen, void* outbuf, size_t outlen,
    const GpaTextureInfo* texinfo, GnmTileMode newtiling
) {
	if (!inbuf || !inlen || !outbuf || !outlen || !texinfo) {
		return GPA_ERR_INVALID_ARGS;
	}

	for (uint32_t a = 0; a < texinfo->numslices; a += 1) {
		for (uint32_t m = 0; m < texinfo->nummips; m += 1) {
			GpaError gerr = gpaTileTextureIndexed(
			    inbuf, inlen, outbuf, outlen, texinfo, newtiling, m,
			    a
			);
			if (gerr != GPA_ERR_OK) {
				return gerr;
			}
		}
	}

	return GPA_ERR_OK;
}
