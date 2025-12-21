#include "gpuaddr_private.h"

#include <string.h>

#define BCDEC_IMPLEMENTATION
#include "bcdec.h"

// these hold intermediate data used to convert formats from and to
typedef struct {
	uint8_t data[BLOCK_SIZE * BLOCK_SIZE * 16];
} TextureBlock;
typedef struct {
	uint8_t data[TILE_SIZE * TILE_SIZE * 16];
} TextureTile;

// converts a BC6h unpacked block, 3 channels (RGB) to 4 channels (RGBA)
static inline TextureBlock block16_rgbtorgba(const TextureBlock* srcblock) {
	TextureBlock res = {0};

	const uint16_t* src = (const uint16_t*)srcblock->data;
	uint16_t* dst = (uint16_t*)res.data;

	for (uint8_t y = 0; y < BLOCK_SIZE; y += 1) {
		for (uint8_t x = 0; x < BLOCK_SIZE; x += 1) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = 0x3c00;  // 1.0 in f16

			src += 3;
			dst += 4;
		}
	}

	return res;
}

static GpaError decomprtile(
    const TextureTile* intile, TextureTile* outtile, const GnmDataFormat infmt,
    const GnmDataFormat outfmt
) {
	const uint32_t tilepitch = gettilepitch(infmt);
	const uint32_t blockpitch = getblockpitch(infmt);
	const uint32_t texelspertileh =
	    tilepitch * BLOCK_SIZE / gnmDfGetTexelsPerElementTall(infmt);

	const uint32_t outtilepitch = gettilepitch(outfmt);
	const uint32_t outblockpitch = getblockpitch(outfmt);
	const uint32_t outnumblockstile =
	    outtilepitch * getelemsperblocktall(outfmt);
	const uint32_t outrowsize =
	    gnmDfGetTotalBytesPerElement(outfmt) * BLOCK_SIZE;

	size_t curoff = 0;
	for (uint8_t y = 0; y < TILE_SIZE / BLOCK_SIZE; y += 1) {
		for (uint8_t x = 0; x < TILE_SIZE / BLOCK_SIZE; x += 1) {
			const uint8_t* inblockdata = &intile->data[curoff];

			// decompress surface
			TextureBlock decblock = {0};
			GnmDataFormat decfmt = infmt;
			switch (infmt.surfacefmt) {
			case GNM_IMG_DATA_FORMAT_BC6:
				decfmt.surfacefmt =
				    GNM_IMG_DATA_FORMAT_16_16_16_16;
				decfmt.chantype = GNM_IMG_NUM_FORMAT_UNORM;
				bcdec_bc6h_half(
				    inblockdata, decblock.data, 4 * 3, 0
				);
				decblock = block16_rgbtorgba(&decblock);
				break;
			case GNM_IMG_DATA_FORMAT_BC7:
				decfmt.surfacefmt = GNM_IMG_DATA_FORMAT_8_8_8_8;
				bcdec_bc7(
				    inblockdata, decblock.data, outblockpitch
				);
				break;
			default:
				return GPA_ERR_UNSUPPORTED;
			}

			if (decfmt.surfacefmt != outfmt.surfacefmt) {
				return GPA_ERR_UNSUPPORTED;
			}

			// write reformatted block to out tile
			const uint32_t dstoff =
			    outblockpitch * x + outnumblockstile * y;
			const uint8_t* cursrc = decblock.data;
			uint8_t* curdst = &outtile->data[dstoff];
			for (uint32_t i = 0; i < BLOCK_SIZE; i += 1) {
				memcpy(curdst, cursrc, outrowsize);
				cursrc += outrowsize;
				curdst += outtilepitch;
			}

			curoff += blockpitch;
		}
		curoff -= blockpitch * (TILE_SIZE / BLOCK_SIZE);
		curoff += texelspertileh;
	}

	return GPA_ERR_OK;
}

static GpaError gettile(
    TextureTile* outtile, const void* inbuf, size_t inlen,
    const GpaTextureInfo* texinfo, const GpaSurfaceIndex* idx, uint32_t x,
    uint32_t y
) {
	const GnmDataFormat fmt = texinfo->fmt;
	const uint32_t elemw = gnmDfGetTexelsPerElementWide(fmt);
	const uint32_t elemh = gnmDfGetTexelsPerElementTall(fmt);
	const uint32_t elemstilewide = TILE_SIZE / elemw;
	const uint32_t elemstiletall = TILE_SIZE / elemh;

	const uint32_t bpp = gnmDfGetTotalBitsPerElement(fmt);
	const uint32_t tilepitchbytes = gettilepitch(fmt);

	const uint32_t totalelemswide =
	    (umax(texinfo->width >> idx->mip, 1) + elemw - 1) / elemw;
	const uint32_t totalelemstall =
	    (umax(texinfo->height >> idx->mip, 1) + elemh - 1) / elemh;

	const uint32_t elemx = umin(elemstilewide * (x + 1), totalelemswide);
	const uint32_t elemy = umin(elemstiletall * (y + 1), totalelemstall);

	const uint32_t numelemsw = elemx - elemstilewide * x;
	const uint32_t numelemsh = elemy - elemstiletall * y;
	const uint32_t byteselemsw = numelemsw * (bpp / 8);
	const uint32_t bytesperrow = totalelemswide * (bpp / 8);

	const uint32_t srcoffset = gpaComputeSurfaceAddrFromCoordLinear(
	    elemstilewide * x, elemstiletall * y, idx->arrayindex,
	    idx->fragment, bpp, texinfo->pitch, texinfo->height,
	    texinfo->numslices, NULL
	);

	if (srcoffset + (numelemsh - 1) * bytesperrow + byteselemsw > inlen) {
		return GPA_ERR_OVERFLOW;
	}
	if ((numelemsh - 1) * tilepitchbytes + byteselemsw >
	    sizeof(TextureTile)) {
		return GPA_ERR_OVERFLOW;
	}

	const uint8_t* cursrc = (const uint8_t*)inbuf + srcoffset;
	uint8_t* curdst = outtile->data;

	for (uint32_t i = 0; i < numelemsh; i += 1) {
		memcpy(curdst, cursrc, byteselemsw);
		cursrc += bytesperrow;
		curdst += tilepitchbytes;
	}

	return GPA_ERR_OK;
}

static GpaError puttile(
    const TextureTile* intile, void* outbuf, size_t outlen,
    const GpaTextureInfo* texinfo, const GpaSurfaceIndex* idx, uint32_t x,
    uint32_t y
) {
	const GnmDataFormat fmt = texinfo->fmt;
	const uint32_t elemw = gnmDfGetTexelsPerElementWide(fmt);
	const uint32_t elemh = gnmDfGetTexelsPerElementTall(fmt);
	const uint32_t elemstilewide = TILE_SIZE / elemw;
	const uint32_t elemstiletall = TILE_SIZE / elemh;
	const uint32_t bpp = gnmDfGetTotalBitsPerElement(fmt);

	const uint32_t tilepitchbytes = gettilepitch(fmt);

	const uint32_t totalelemswide =
	    (umax(texinfo->width >> idx->mip, 1) + elemw - 1) / elemw;
	const uint32_t totalelemstall =
	    (umax(texinfo->height >> idx->mip, 1) + elemh - 1) / elemh;

	const uint32_t elemx = umin(elemstilewide * (x + 1), totalelemswide);
	const uint32_t elemy = umin(elemstiletall * (y + 1), totalelemstall);

	const uint32_t numelemsw = elemx - elemstilewide * x;
	const uint32_t numelemsh = elemy - elemstiletall * y;
	const uint32_t byteselemsw = numelemsw * (bpp / 8);
	const uint32_t outbytesperrow = totalelemswide * (bpp / 8);

	const uint32_t dstoffset = gpaComputeSurfaceAddrFromCoordLinear(
	    elemstilewide * x, elemstiletall * y, idx->arrayindex,
	    idx->fragment, bpp, texinfo->width, texinfo->height,
	    texinfo->numslices, NULL
	);

	if ((numelemsh - 1) * tilepitchbytes + byteselemsw >
	    sizeof(TextureTile)) {
		return GPA_ERR_OVERFLOW;
	}
	if (dstoffset + (numelemsh - 1) * outbytesperrow + byteselemsw >
	    outlen) {
		return GPA_ERR_OVERFLOW;
	}

	const uint8_t* cursrc = intile->data;
	uint8_t* curdst = (uint8_t*)outbuf + dstoffset;

	for (uint32_t i = 0; i < numelemsh; i += 1) {
		memcpy(curdst, cursrc, byteselemsw);
		cursrc += tilepitchbytes;
		curdst += outbytesperrow;
	}

	return GPA_ERR_OK;
}

static GpaError decomprsurf(
    void* outbuf, size_t outlen, const void* inbuf, size_t inlen,
    const GpaTextureInfo* texinfo, const GpaSurfaceIndex* idx,
    const GnmDataFormat decfmt
) {
	const uint32_t numtileswide =
	    (umax(texinfo->width >> idx->mip, 1) + TILE_SIZE - 1) / TILE_SIZE;
	const uint32_t numtilestall =
	    (umax(texinfo->height >> idx->mip, 1) + TILE_SIZE - 1) / TILE_SIZE;

	GpaTextureInfo dectexinfo = *texinfo;
	dectexinfo.fmt = decfmt;

	for (uint32_t h = 0; h < numtilestall; h += 1) {
		for (uint32_t w = 0; w < numtileswide; w += 1) {
			TextureTile srctile = {0};
			GpaError res =
			    gettile(&srctile, inbuf, inlen, texinfo, idx, w, h);
			if (res != GPA_ERR_OK) {
				return res;
			}

			TextureTile dst_tile = {0};
			res = decomprtile(
			    &srctile, &dst_tile, texinfo->fmt, decfmt
			);
			if (res != GPA_ERR_OK) {
				return res;
			}

			res = puttile(
			    &dst_tile, outbuf, outlen, &dectexinfo, idx, w, h
			);
			if (res != GPA_ERR_OK) {
				return res;
			}
		}
	}

	return GPA_ERR_OK;
}

static inline GnmDataFormat getdecompressfmt(const GnmDataFormat fmt) {
	switch (fmt.surfacefmt) {
	case GNM_IMG_DATA_FORMAT_BC6:
		return GNM_FMT_R16G16B16A16_UNORM;
	case GNM_IMG_DATA_FORMAT_BC7:
		return GNM_FMT_R8G8B8A8_SRGB;
	default:
		return GNM_FMT_INVALID;
	}
}

GpaError gpaGetDecompressedSize(
    uint64_t* outsize, const GpaTextureInfo* texinfo
) {
	if (!outsize || !texinfo) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaTextureInfo dectexinfo = *texinfo;
	dectexinfo.fmt = getdecompressfmt(texinfo->fmt);

	GpaTilingParams tp = {0};
	GpaError err = gpaTpInit(&tp, &dectexinfo, 0, 0);
	if (err != GPA_ERR_OK) {
		return err;
	}

	GpaSurfaceInfo surfinfo = {0};
	err = gpaComputeSurfaceInfo(&surfinfo, &tp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	*outsize = surfinfo.surfacesize;
	return GPA_ERR_OK;
}

GpaError gpaDecompressTexture(
    void* outbuf, uint64_t outlen, const void* inbuf, uint64_t inlen,
    const GpaTextureInfo* texinfo, GnmDataFormat* outfmt
) {
	if (!outbuf || !outlen || !inbuf || !inlen || !texinfo) {
		return GPA_ERR_INVALID_ARGS;
	}

	if (!gnmDfIsBlockCompressed(texinfo->fmt)) {
		return GPA_ERR_NOT_COMPRESSED;
	}

	const GnmDataFormat decfmt = getdecompressfmt(texinfo->fmt);
	if (decfmt.asuint == GNM_FMT_INVALID.asuint) {
		return GPA_ERR_UNSUPPORTED;
	}

	for (uint32_t a = 0; a < texinfo->numslices; a += 1) {
		for (uint32_t d = 0; d < texinfo->depth; d += 1) {
			for (uint32_t m = 0; m < texinfo->nummips; m += 1) {
				for (uint32_t fr = 0; fr < texinfo->numfrags;
				     fr += 1) {
					const GpaSurfaceIndex idx = {
					    .arrayindex = a,
					    .depth = d,
					    .face = 0,
					    .fragment = fr,
					    .mip = m,
					    .sample = 0,
					};

					GpaError err = decomprsurf(
					    outbuf, outlen, inbuf, inlen,
					    texinfo, &idx, decfmt
					);
					if (err != GPA_ERR_OK) {
						return err;
					}
				}
			}
		}
	}

	if (outfmt) {
		*outfmt = decfmt;
	}
	return GPA_ERR_OK;
}
