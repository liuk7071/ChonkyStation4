#include "gpuaddr_private.h"

#include <math.h>

static inline void ComputeMipLevel(
    uint32_t* outwidth, uint32_t* outheight, const GpaTilingParams* params
) {
	uint32_t width = params->linearwidth;
	uint32_t height = params->linearheight;

	if (params->isblockcompressed) {
		if (params->miplevel == 0) {
			// DXTn's level 0 must be multiple of 4
			// But there are exceptions:
			// 1. Internal surface creation in hostblt/vsblt/etc...
			// 2. Runtime doesn't reject ATI1/ATI2 whose
			// width/height are not multiple of 4
			width = PowTwoAlign32(width, 4);
			height = PowTwoAlign32(height, 4);
		}
	}

	// basePitch is calculated from level 0 so we only check this
	// for mipLevel > 0
	if (params->miplevel > 0 && params->basetiledpitch != 0) {
		width = umax(1u, params->basetiledpitch >> params->miplevel);
	}

	// pow2Pad is done in PostComputeMipLevel

	*outwidth = width;
	*outheight = height;
}

static inline void AdjustPitchAlignment(
    GpaSurfaceFlags flags,  ///< [in] Surface flags
    uint32_t* pPitchAlign   ///< [out] Pointer to pitch alignment
) {
	// Display engine hardwires lower 5 bit of GRPH_PITCH to ZERO
	// which means 32 pixel alignment Maybe it will be fixed in
	// future but let's make it general for now.
	if (flags.display || flags.overlay) {
		*pPitchAlign = PowTwoAlign32(*pPitchAlign, 32);

		if (flags.display) {
			*pPitchAlign = umax(1, *pPitchAlign);
			/**pPitchAlign =
			    umax(m_minPitchAlignPixels, *pPitchAlign);*/
		}
	}
}

static void ComputeSurfaceAlignmentsLinear(
    GnmArrayMode arrayMode,  ///< [in] tile mode
    uint32_t bpp,	     ///< [in] bits per pixel
    GpaSurfaceFlags flags,   ///< [in] surface flags
    uint32_t* pBaseAlign,    ///< [out] base address alignment in bytes
    uint32_t* pPitchAlign,   ///< [out] pitch alignment in pixels
    uint32_t* pHeightAlign   ///< [out] height alignment in pixels
) {
	switch (arrayMode) {
	case GNM_ARRAY_LINEAR_GENERAL:
		//
		// The required base alignment and pitch and height
		// granularities is to 1 element.
		//
		*pBaseAlign = (bpp > 8) ? bpp / 8 : 1;
		*pPitchAlign = 1;
		*pHeightAlign = 1;
		break;
	case GNM_ARRAY_LINEAR_ALIGNED:
		//
		// The required alignment for base is the pipe
		// interleave size. The required granularity for pitch
		// is hwl dependent. The required granularity for height
		// is one row.
		//
		*pBaseAlign = PIPE_INTERLEAVE_BYTES;
		*pPitchAlign = umax(8u, 64 / BitsToBytes32(bpp));
		*pHeightAlign = 1;
		break;
	default:
		*pBaseAlign = 1;
		*pPitchAlign = 1;
		*pHeightAlign = 1;
		break;
	}

	AdjustPitchAlignment(flags, pPitchAlign);
}

static void ComputeSurfaceAlignmentsMicroTiled(
    GpaSurfaceFlags flags,  ///< [in] surface flags
    uint32_t* pBaseAlign,   ///< [out] base address alignment in bytes
    uint32_t* pPitchAlign,  ///< [out] pitch alignment in pixels
    uint32_t* pHeightAlign  ///< [out] height alignment in pixels
) {
	//
	// The required alignment for base is the pipe interleave size.
	//
	*pBaseAlign = PIPE_INTERLEAVE_BYTES;
	*pPitchAlign = MicroTileWidth;
	*pHeightAlign = MicroTileHeight;

	AdjustPitchAlignment(flags, pPitchAlign);
}

static GpaError HwlReduceBankWidthHeight(
    uint32_t tileSize,	       ///< [in] tile size
    uint32_t bpp,	       ///< [in] bits per pixel
    GpaSurfaceFlags flags,     ///< [in] surface flags
    uint32_t numSamples,       ///< [in] number of samples
    uint32_t bankHeightAlign,  ///< [in] bank height alignment
    uint32_t pipes,	       ///< [in] pipes
    GpaTileInfo* pTileInfo     ///< [in,out] bank structure.
) {
	uint32_t macroAspectAlign;
	bool valid = true;

	uint32_t bankWidth = (1 << pTileInfo->bankwidth);
	uint32_t bankHeight = (1 << pTileInfo->bankheight);
	uint32_t macroAspectRatio = (1 << pTileInfo->macroaspectratio);

	if (tileSize * bankWidth * bankHeight > DRAM_ROW_SIZE) {
		bool stillGreater = true;

		// Try reducing bankWidth first
		if (stillGreater && bankWidth > 1) {
			while (stillGreater && bankWidth > 0) {
				bankWidth >>= 1;

				if (bankWidth == 0) {
					bankWidth = 1;
					break;
				}

				stillGreater =
				    tileSize * bankWidth * bankHeight >
				    DRAM_ROW_SIZE;
			}

			// bankWidth is reduced above, so we need to
			// recalculate bankHeight and ratio
			bankHeightAlign = umax(
			    1u, PIPE_INTERLEAVE_BYTES * BANK_INTERLEAVE /
				    (tileSize * bankWidth)
			);

			// We cannot increase bankHeight so just assert
			// this case.
			if ((bankHeight % bankHeightAlign) != 0) {
				return GPA_ERR_INTERNAL_ERROR;
			}

			if (numSamples == 1) {
				macroAspectAlign = umax(
				    1u, PIPE_INTERLEAVE_BYTES *
					    BANK_INTERLEAVE /
					    (tileSize * pipes * bankWidth)
				);
				macroAspectRatio = PowTwoAlign32(
				    macroAspectRatio, macroAspectAlign
				);
			}
		}

		// Early quit bank_height degradation for "64" bit z
		// buffer
		if (flags.depthtarget && bpp >= 64) {
			stillGreater = false;
		}

		// Then try reducing bankHeight
		if (stillGreater && bankHeight > bankHeightAlign) {
			while (stillGreater && bankHeight > bankHeightAlign) {
				bankHeight >>= 1;

				if (bankHeight < bankHeightAlign) {
					bankHeight = bankHeightAlign;
					break;
				}

				stillGreater =
				    tileSize * bankWidth * bankHeight >
				    DRAM_ROW_SIZE;
			}
		}

		valid = !stillGreater;
	}

	return valid ? GPA_ERR_OK : GPA_ERR_UNSUPPORTED;
}

static GpaError ComputeSurfaceAlignmentsMacroTiled(
    GnmArrayMode tileMode,  ///< [in] tile mode
    uint32_t bpp,	    ///< [in] bits per pixel
    GpaSurfaceFlags flags,  ///< [in] surface flags
    uint32_t mipLevel,	    ///< [in] mip level
    uint32_t numSamples,    ///< [in] number of samples
    GpaSurfaceInfo* pOut    ///< [in,out] Surface output
) {
	uint32_t macroTileWidth;
	uint32_t macroTileHeight;

	uint32_t tileSize;
	uint32_t bankHeightAlign;
	uint32_t macroAspectAlign;

	uint32_t thickness = gpaGetMicroTileThickness(tileMode);
	uint32_t pipes = gpaGetPipeCount(pOut->tileinfo.pipeconfig);

	//
	// Align bank height first according to latest h/w spec
	//

	const uint32_t banks = 2 << pOut->tileinfo.banks;
	const uint32_t bankWidth = (1 << pOut->tileinfo.bankwidth);
	uint32_t bankHeight = (1 << pOut->tileinfo.bankheight);
	uint32_t macroAspectRatio = (1 << pOut->tileinfo.macroaspectratio);
	const uint32_t tileSplitBytes =
	    GetTileSplitBytes(pOut->tileinfo.tilesplit, bpp, thickness);

	// tile_size = MIN(tile_split, 64 * tile_thickness *
	// element_bytes * num_samples)
	tileSize = umin(
	    tileSplitBytes, BitsToBytes32(64 * thickness * bpp * numSamples)
	);

	// bank_height_align =
	// MAX(1, (pipe_interleave_bytes *
	// bank_interleave)/(tile_size*bank_width))
	bankHeightAlign = umax(
	    1u, PIPE_INTERLEAVE_BYTES * BANK_INTERLEAVE / (tileSize * bankWidth)
	);

	bankHeight = PowTwoAlign32(bankHeight, bankHeightAlign);

	// num_pipes * bank_width * macro_tile_aspect >=
	// (pipe_interleave_size * bank_interleave) / tile_size
	if (numSamples == 1) {
		// this restriction is only for mipmap (mipmap's
		// numSamples must be 1)
		macroAspectAlign = umax(
		    1u, PIPE_INTERLEAVE_BYTES * BANK_INTERLEAVE /
			    (tileSize * pipes * bankWidth)
		);
		macroAspectRatio =
		    PowTwoAlign32(macroAspectRatio, macroAspectAlign);
	}

	// Sony's library ignores any failure, so do the same
	GpaError err = HwlReduceBankWidthHeight(
	    tileSize, bpp, flags, numSamples, bankHeightAlign, pipes,
	    &pOut->tileinfo
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	//
	// The required granularity for pitch is the macro tile
	// width.
	//
	macroTileWidth = MicroTileWidth * bankWidth * pipes * macroAspectRatio;

	pOut->pitchalign = macroTileWidth;
	pOut->blockwidth = macroTileWidth;

	AdjustPitchAlignment(flags, &pOut->pitchalign);

	//
	// The required granularity for height is the macro tile
	// height.
	//
	macroTileHeight =
	    MicroTileHeight * bankHeight * banks / macroAspectRatio;

	pOut->heightalign = macroTileHeight;
	pOut->blockheight = macroTileHeight;

	//
	// Compute base alignment
	//
	pOut->basealign = pipes * bankWidth * banks * bankHeight * tileSize;

	if ((mipLevel == 0) && (flags.prt)) {
		uint32_t macroTileSize =
		    pOut->blockwidth * pOut->blockheight * numSamples * bpp / 8;

		if (macroTileSize < PrtTileSize) {
			uint32_t numMacroTiles = PrtTileSize / macroTileSize;

			if ((PrtTileSize % macroTileSize) != 0) {
				return GPA_ERR_INTERNAL_ERROR;
			}

			pOut->pitchalign *= numMacroTiles;
			pOut->basealign *= numMacroTiles;
		}
	}

	return GPA_ERR_OK;
}

static GpaError PadDimensions(
    GnmArrayMode arrayMode,	   ///< [in] tile mode
    uint32_t bpp,		   ///< [in] bits per pixel
    GpaSurfaceFlags flags,	   ///< [in] surface flags
    uint32_t numSamples,	   ///< [in] number of samples
    const GpaTileInfo* pTileInfo,  ///< [in] bank structure.
    uint32_t padDims,		   ///< [in] Dimensions to pad valid value 1,2,3
    uint32_t mipLevel,		   ///< [in] MipLevel
    uint32_t* pPitch,		   ///< [in,out] pitch in pixels
    uint32_t* pPitchAlign,	   ///< [in,out] pitch align could be changed in
				   ///< HwlPadDimensions
    uint32_t* pHeight,		   ///< [in,out] height in pixels
    uint32_t heightAlign,	   ///< [in] height alignment
    uint32_t* pSlices,		   ///< [in,out] number of slices
    uint32_t sliceAlign,	   ///< [in] number of slice alignment
    GnmGpuMode mingpumode	   ///< [in] min GPU mode
) {
	uint32_t pitchAlign = *pPitchAlign;
	uint32_t thickness = gpaGetMicroTileThickness(arrayMode);

	if (padDims > 3) {
		return GPA_ERR_INVALID_ARGS;
	}

	//
	// Override padding for mip levels
	//
	if (mipLevel > 0) {
		if (flags.cube) {
			// for cubemap, we only pad when client call
			// with 6 faces as an identity
			if (*pSlices > 1) {
				padDims = 3;  // we should pad cubemap
					      // sub levels when we
					      // treat it as 3d texture
			} else {
				padDims = 2;
			}
		}
	}

	// Any possibilities that padDims is 0?
	if (padDims == 0) {
		padDims = 3;
	}

	if (IsPow2(pitchAlign)) {
		*pPitch = PowTwoAlign32((*pPitch), pitchAlign);
	} else	// add this code to pass unit test, r600 linear mode is
		// not align bpp to pow2 for linear
	{
		*pPitch += pitchAlign - 1;
		*pPitch /= pitchAlign;
		*pPitch *= pitchAlign;
	}

	if (padDims > 1) {
		if (IsPow2(heightAlign)) {
			*pHeight = PowTwoAlign32((*pHeight), heightAlign);
		} else {
			*pHeight += heightAlign - 1;
			*pHeight /= heightAlign;
			*pHeight *= heightAlign;
		}
	}

	if (padDims > 2 || thickness > 1) {
		// for cubemap single face, we do not pad slices.
		// if we pad it, the slice number should be set to 6 and
		// current mip level > 1
		if (flags.cube && flags.cubeasarray) {
			*pSlices = NextPow2(*pSlices);
		}

		// normal 3D texture or arrays or cubemap has a thick
		// mode? (Just pass unit test)
		if (thickness > 1) {
			*pSlices = PowTwoAlign32((*pSlices), sliceAlign);
		}
	}

	if (mingpumode == GNM_GPU_NEO && (numSamples > 1) && (mipLevel == 0) &&
	    gpaIsMacroTiled(arrayMode)) {
		const uint32_t tileSplitBytes =
		    GetTileSplitBytes(pTileInfo->tilesplit, bpp, thickness);
		uint32_t tileSizePerSample =
		    BitsToBytes32(bpp * MicroTileWidth * MicroTileHeight);
		uint32_t samplesPerSplit = tileSplitBytes / tileSizePerSample;

		if (samplesPerSplit < numSamples) {
			uint32_t dccFastClearByteAlign =
			    gpaGetPipeCount(pTileInfo->pipeconfig) *
			    PIPE_INTERLEAVE_BYTES * 256;
			uint32_t bytesPerSplit = BitsToBytes32(
			    (*pPitch) * (*pHeight) * bpp * samplesPerSplit
			);

			if (!IsPow2(dccFastClearByteAlign)) {
				return GPA_ERR_INTERNAL_ERROR;
			}

			if (0 !=
			    (bytesPerSplit & (dccFastClearByteAlign - 1))) {
				uint32_t dccFastClearPixelAlign =
				    dccFastClearByteAlign / BitsToBytes32(bpp) /
				    samplesPerSplit;
				uint32_t macroTilePixelAlign =
				    (*pPitchAlign) * heightAlign;

				if ((dccFastClearPixelAlign >=
				     macroTilePixelAlign) &&
				    ((dccFastClearPixelAlign %
				      macroTilePixelAlign) == 0)) {
					uint32_t
					    dccFastClearPitchAlignInMacroTile =
						dccFastClearPixelAlign /
						macroTilePixelAlign;
					uint32_t heightInMacroTile =
					    (*pHeight) / heightAlign;

					while (
					    (heightInMacroTile > 1) &&
					    ((heightInMacroTile % 2) == 0) &&
					    (dccFastClearPitchAlignInMacroTile >
					     1) &&
					    ((dccFastClearPitchAlignInMacroTile %
					      2) == 0)
					) {
						heightInMacroTile >>= 1;
						dccFastClearPitchAlignInMacroTile >>=
						    1;
					}

					uint32_t dccFastClearPitchAlignInPixels =
					    (*pPitchAlign) *
					    dccFastClearPitchAlignInMacroTile;

					if (IsPow2(
						dccFastClearPitchAlignInPixels
					    )) {
						*pPitch = PowTwoAlign32(
						    (*pPitch),
						    dccFastClearPitchAlignInPixels
						);
					} else {
						*pPitch +=
						    (dccFastClearPitchAlignInPixels -
						     1);
						*pPitch /=
						    dccFastClearPitchAlignInPixels;
						*pPitch *=
						    dccFastClearPitchAlignInPixels;
					}

					*pPitchAlign =
					    dccFastClearPitchAlignInPixels;
				}
			}
		}
	}

	return GPA_ERR_OK;
}

static uint64_t HwlGetSizeAdjustmentLinear(
    GnmArrayMode arrayMode,  ///< [in] tile mode
    uint32_t bpp,	     ///< [in] bits per pixel
    uint32_t numSamples,     ///< [in] number of samples
    uint32_t pitchAlign,     ///< [in] pitch alignment
    uint32_t* pPitch,	     ///< [in,out] pointer to pitch
    uint32_t* pHeight,	     ///< [in,out] pointer to height
    uint32_t* pHeightAlign   ///< [in,out] pointer to height align
) {
	uint64_t sliceSize;
	if (arrayMode == GNM_ARRAY_LINEAR_GENERAL) {
		sliceSize = BitsToBytes64(
		    (uint64_t)(*pPitch) * (*pHeight) * bpp * numSamples
		);
	} else {
		uint32_t pitch = *pPitch;
		uint32_t height = *pHeight;

		uint32_t pixelsPerPipeInterleave =
		    PIPE_INTERLEAVE_BYTES / BitsToBytes32(bpp);
		uint32_t sliceAlignInPixel =
		    pixelsPerPipeInterleave < 64 ? 64 : pixelsPerPipeInterleave;

		// numSamples should be 1 in real cases (no MSAA for
		// linear but TGL may pass non 1 value)
		uint64_t pixelPerSlice = (uint64_t)(pitch)*height * numSamples;

		while (pixelPerSlice % sliceAlignInPixel) {
			pitch += pitchAlign;
			pixelPerSlice = (uint64_t)(pitch)*height * numSamples;
		}

		*pPitch = pitch;

		uint32_t heightAlign = 1;

		while ((pitch * heightAlign) % sliceAlignInPixel) {
			heightAlign++;
		}

		*pHeightAlign = heightAlign;

		sliceSize = BitsToBytes64(pixelPerSlice * bpp);
	}

	return sliceSize;
}

static uint64_t HwlGetSizeAdjustmentMicroTiled(
    uint32_t bpp,	  ///< [in] bits per pixel
    uint32_t numSamples,  ///< [in] number of samples
    uint32_t* pPitch,	  ///< [in,out] pointer to pitch
    uint32_t* pHeight	  ///< [in,out] pointer to height
) {
	uint64_t logicalSliceSize;
	// uint64_t physicalSliceSize;

	uint32_t pitch = *pPitch;
	uint32_t height = *pHeight;

	// Logical slice: pitch * height * bpp * numSamples (no 1D MSAA
	// so actually numSamples == 1)
	logicalSliceSize =
	    BitsToBytes64((uint64_t)pitch * height * bpp * numSamples);

	// Physical slice: multiplied by thickness
	// physicalSliceSize = logicalSliceSize * thickness;

	//
	// R800 will always pad physical slice size to baseAlign which
	// is pipe_interleave_bytes
	//
	// ADDR_ASSERT((physicalSliceSize % baseAlign) == 0);

	return logicalSliceSize;
}

static GpaError ComputeSurfaceInfoLinear(
    const GpaTilingParams* params,  ///< [in] Input structure
    GpaSurfaceInfo* out,	    ///< [out] Output structure
    uint32_t padDims		    ///< [in] Dimensions to padd
) {
	uint32_t expPitch = params->linearwidth;
	uint32_t expHeight = params->linearheight;
	uint32_t expNumSlices = params->lineardepth;

	// No linear MSAA on real H/W, keep this for TGL
	uint32_t numSamples = params->numfragsperpixel;

	const uint32_t microTileThickness = 1;
	const GnmArrayMode arrayMode = gpaGetArrayMode(params->tilemode);

	//
	// Compute the surface alignments.
	//
	ComputeSurfaceAlignmentsLinear(
	    arrayMode, params->bitsperfrag, params->surfaceflags,
	    &out->basealign, &out->pitchalign, &out->heightalign
	);

	if (arrayMode == GNM_ARRAY_LINEAR_GENERAL &&
	    params->surfaceflags.colortarget && (params->linearheight > 1)) {
		// When linear_general surface is accessed in multiple
		// lines, it requires 8 pixels in pitch alignment since
		// PITCH_TILE_MAX is in unit of 8 pixels. It is OK if it
		// is accessed per line.
		if ((params->linearwidth % 8) != 0) {
			return GPA_ERR_INTERNAL_ERROR;
		}
	}

	out->depthalign = microTileThickness;

	//
	// Pad pitch and height to the required granularities.
	//
	GpaError err = PadDimensions(
	    arrayMode, params->bitsperfrag, params->surfaceflags, numSamples,
	    &out->tileinfo, padDims, params->miplevel, &expPitch,
	    &out->pitchalign, &expHeight, out->heightalign, &expNumSlices,
	    microTileThickness, params->mingpumode
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	//
	// Adjust per HWL
	//

	uint64_t logicalSliceSize = HwlGetSizeAdjustmentLinear(
	    arrayMode, params->bitsperfrag, numSamples, out->pitchalign,
	    &expPitch, &expHeight, &out->heightalign
	);

	out->pitch = expPitch;
	out->height = expHeight;
	out->depth = expNumSlices;

	out->surfacesize = logicalSliceSize * expNumSlices;

	out->tilemode = params->tilemode;

	return GPA_ERR_OK;
}

static GnmArrayMode HwlDegradeThickTileMode(
    GnmArrayMode baseTileMode,	///< [in] base tile mode
    uint32_t numSlices,		///< [in] current number of slices
    uint32_t* pBytesPerTile	///< [in,out] pointer to bytes per slice
) {
	// if pBytesPerTile is NULL, this is a don't-care....
	uint32_t bytesPerTile = pBytesPerTile != NULL ? *pBytesPerTile : 64;

	GnmArrayMode expTileMode = baseTileMode;
	switch (baseTileMode) {
	case GNM_ARRAY_1D_TILED_THICK:
		expTileMode = GNM_ARRAY_1D_TILED_THIN1;
		bytesPerTile >>= 2;
		break;
	case GNM_ARRAY_2D_TILED_THICK:
		expTileMode = GNM_ARRAY_2D_TILED_THIN1;
		bytesPerTile >>= 2;
		break;
	case GNM_ARRAY_3D_TILED_THICK:
		expTileMode = GNM_ARRAY_3D_TILED_THIN1;
		bytesPerTile >>= 2;
		break;
	case GNM_ARRAY_2D_TILED_XTHICK:
		if (numSlices < ThickTileThickness) {
			expTileMode = GNM_ARRAY_2D_TILED_THIN1;
			bytesPerTile >>= 3;
		} else {
			expTileMode = GNM_ARRAY_2D_TILED_THICK;
			bytesPerTile >>= 1;
		}
		break;
	case GNM_ARRAY_3D_TILED_XTHICK:
		if (numSlices < ThickTileThickness) {
			expTileMode = GNM_ARRAY_3D_TILED_THIN1;
			bytesPerTile >>= 3;
		} else {
			expTileMode = GNM_ARRAY_3D_TILED_THICK;
			bytesPerTile >>= 1;
		}
		break;
	default:
		break;
	}

	if (pBytesPerTile != NULL) {
		*pBytesPerTile = bytesPerTile;
	}

	return expTileMode;
}

static GnmArrayMode ComputeSurfaceMipLevelTileMode(
    GnmArrayMode baseTileMode,	  ///< [in] base tile mode
    uint32_t bpp,		  ///< [in] bits per pixels
    uint32_t pitch,		  ///< [in] current level pitch
    uint32_t height,		  ///< [in] current level height
    uint32_t numSlices,		  ///< [in] current number of slices
    uint32_t numSamples,	  ///< [in] number of samples
    uint32_t pitchAlign,	  ///< [in] pitch alignment
    uint32_t heightAlign,	  ///< [in] height alignment
    const GpaTileInfo* pTileInfo  ///< [in] ptr to bank structure
) {
	// uint64_t bytesPerSlice;
	uint32_t bytesPerTile;

	GnmArrayMode expTileMode = baseTileMode;
	uint32_t microTileThickness = gpaGetMicroTileThickness(expTileMode);
	uint32_t interleaveSize = PIPE_INTERLEAVE_BYTES * BANK_INTERLEAVE;

	//
	// Compute the size of a slice.
	//
	/*bytesPerSlice =
	    BitsToBytes64((uint64_t)pitch * height * bpp * numSamples);*/
	bytesPerTile = BitsToBytes32(
	    MicroTilePixels * microTileThickness * NextPow2(bpp) * numSamples
	);

	//
	// Reduce tiling mode from thick to thin if the number of slices
	// is less than the micro tile thickness.
	//
	if (numSlices < microTileThickness) {
		expTileMode = HwlDegradeThickTileMode(
		    expTileMode, numSlices, &bytesPerTile
		);
	}

	const uint32_t bankWidth = (1 << pTileInfo->bankwidth);
	const uint32_t bankHeight = (1 << pTileInfo->bankheight);
	const uint32_t macroAspectRatio = (1 << pTileInfo->macroaspectratio);
	const uint32_t tileSplitBytes =
	    GetTileSplitBytes(pTileInfo->tilesplit, bpp, microTileThickness);
	if (bytesPerTile > tileSplitBytes) {
		bytesPerTile = tileSplitBytes;
	}

	uint32_t threshold1 = bytesPerTile *
			      gpaGetPipeCount(pTileInfo->pipeconfig) *
			      bankWidth * macroAspectRatio;

	uint32_t threshold2 = bytesPerTile * bankWidth * bankHeight;

	//
	// Reduce the tile mode from 2D/3D to 1D in following conditions
	//
	switch (expTileMode) {
	case GNM_ARRAY_2D_TILED_THIN1:	// fall through
	case GNM_ARRAY_3D_TILED_THIN1:
	case GNM_ARRAY_PRT_TILED_THIN1:
	case GNM_ARRAY_PRT_2D_TILED_THIN1:
	case GNM_ARRAY_PRT_3D_TILED_THIN1:
		if ((pitch < pitchAlign) || (height < heightAlign) ||
		    (interleaveSize > threshold1) ||
		    (interleaveSize > threshold2)) {
			expTileMode = GNM_ARRAY_1D_TILED_THIN1;
		}
		break;
	case GNM_ARRAY_2D_TILED_THICK:	// fall through
	case GNM_ARRAY_3D_TILED_THICK:
	case GNM_ARRAY_2D_TILED_XTHICK:
	case GNM_ARRAY_3D_TILED_XTHICK:
	case GNM_ARRAY_PRT_TILED_THICK:
	case GNM_ARRAY_PRT_2D_TILED_THICK:
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		if ((pitch < pitchAlign) || (height < heightAlign)) {
			expTileMode = GNM_ARRAY_1D_TILED_THICK;
		}
		break;
	default:
		break;
	}

	return expTileMode;
}

static GpaError ComputeSurfaceInfoMicroTiled(
    const GpaTilingParams* pIn,	 ///< [in] Input structure
    GpaSurfaceInfo* pOut,	 ///< [out] Output structure
    uint32_t padDims,		 ///< [in] Dimensions to padd
    GnmArrayMode expTileMode	 ///< [in] Expected tile mode
) {
	uint32_t microTileThickness;
	uint32_t expPitch = pIn->linearwidth;
	uint32_t expHeight = pIn->linearheight;
	uint32_t expNumSlices = pIn->lineardepth;

	// No 1D MSAA on real H/W, keep this for TGL
	uint32_t numSamples = pIn->numfragsperpixel;

	//
	// Compute the micro tile thickness.
	//
	microTileThickness = gpaGetMicroTileThickness(expTileMode);

	//
	// Extra override for mip levels
	//
	if (pIn->miplevel > 0) {
		//
		// Reduce tiling mode from thick to thin if the number
		// of slices is less than the micro tile thickness.
		//
		if ((expTileMode == GNM_ARRAY_1D_TILED_THICK) &&
		    (expNumSlices < ThickTileThickness)) {
			expTileMode = HwlDegradeThickTileMode(
			    GNM_ARRAY_1D_TILED_THICK, expNumSlices, NULL
			);
			if (expTileMode != GNM_ARRAY_1D_TILED_THICK) {
				microTileThickness = 1;
			}
		}
	}

	//
	// Compute the surface restrictions.
	//
	ComputeSurfaceAlignmentsMicroTiled(
	    pIn->surfaceflags, &pOut->basealign, &pOut->pitchalign,
	    &pOut->heightalign
	);

	pOut->depthalign = microTileThickness;

	//
	// Pad pitch and height to the required granularities.
	// Compute surface size.
	// Return parameters.
	//
	PadDimensions(
	    expTileMode, pIn->bitsperfrag, pIn->surfaceflags, numSamples,
	    &pOut->tileinfo, padDims, pIn->miplevel, &expPitch,
	    &pOut->pitchalign, &expHeight, pOut->heightalign, &expNumSlices,
	    microTileThickness, pIn->mingpumode
	);

	//
	// Get HWL specific pitch adjustment
	//
	uint64_t logicalSliceSize = HwlGetSizeAdjustmentMicroTiled(
	    pIn->bitsperfrag, numSamples, &expPitch, &expHeight
	);

	pOut->pitch = expPitch;
	pOut->height = expHeight;
	pOut->depth = expNumSlices;

	pOut->surfacesize = logicalSliceSize * expNumSlices;

	GpaError err =
	    gpaAdjustTileMode(&pOut->tilemode, pIn->tilemode, expTileMode);
	if (err != GPA_ERR_OK) {
		return err;
	}

	return GPA_ERR_OK;
}

static GpaError ComputeSurfaceInfoMacroTiled(
    const GpaTilingParams* pIn,	 ///< [in] Input structure
    GpaSurfaceInfo* pOut,	 ///< [out] Output structure
    uint32_t padDims,		 ///< [in] Dimensions to padd
    GnmArrayMode expTileMode	 ///< [in] Expected tile mode
) {
	GnmArrayMode origTileMode = expTileMode;
	uint32_t microTileThickness;

	uint32_t paddedPitch;
	uint32_t paddedHeight;
	uint64_t bytesPerSlice;

	uint32_t expPitch = pIn->linearwidth;
	uint32_t expHeight = pIn->linearheight;
	uint32_t expNumSlices = pIn->lineardepth;

	uint32_t numSamples = pIn->numfragsperpixel;

	//
	// Compute the surface restrictions as base SanityCheckMacroTiled is
	// called in ComputeSurfaceAlignmentsMacroTiled
	//
	GpaError err = ComputeSurfaceAlignmentsMacroTiled(
	    expTileMode, pIn->bitsperfrag, pIn->surfaceflags, pIn->miplevel,
	    numSamples, pOut
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	//
	// Compute the micro tile thickness.
	//
	microTileThickness = gpaGetMicroTileThickness(expTileMode);

	//
	// Find the correct tiling mode for mip levels
	//
	if (pIn->miplevel > 0) {
		//
		// Try valid tile mode
		//
		expTileMode = ComputeSurfaceMipLevelTileMode(
		    expTileMode, pIn->bitsperfrag, expPitch, expHeight,
		    expNumSlices, numSamples, pOut->blockwidth,
		    pOut->blockheight, &pOut->tileinfo
		);

		if (!gpaIsMacroTiled(expTileMode))  // Downgraded to micro-tiled
		{
			return ComputeSurfaceInfoMicroTiled(
			    pIn, pOut, padDims, expTileMode
			);
		} else if (microTileThickness != gpaGetMicroTileThickness(expTileMode)) {
			//
			// Re-compute if thickness changed since bank-height may
			// be changed!
			//
			return ComputeSurfaceInfoMacroTiled(
			    pIn, pOut, padDims, expTileMode
			);
		}
	}

	paddedPitch = expPitch;
	paddedHeight = expHeight;

	//
	// Re-cal alignment
	//
	if (expTileMode !=
	    origTileMode)  // Tile mode is changed but still macro-tiled
	{
		err = ComputeSurfaceAlignmentsMacroTiled(
		    expTileMode, pIn->bitsperfrag, pIn->surfaceflags,
		    pIn->miplevel, numSamples, pOut
		);
		if (err != GPA_ERR_OK) {
			return err;
		}
	}

	//
	// Do padding
	//
	PadDimensions(
	    expTileMode, pIn->bitsperfrag, pIn->surfaceflags, numSamples,
	    &pOut->tileinfo, padDims, pIn->miplevel, &paddedPitch,
	    &pOut->pitchalign, &paddedHeight, pOut->heightalign, &expNumSlices,
	    microTileThickness, pIn->mingpumode
	);

	pOut->pitch = paddedPitch;
	pOut->height = paddedHeight;
	pOut->depth = expNumSlices;

	//
	// Compute the size of a slice.
	//
	bytesPerSlice = BitsToBytes64(
	    (uint64_t)paddedPitch * paddedHeight * NextPow2(pIn->bitsperfrag) *
	    numSamples
	);

	pOut->surfacesize = bytesPerSlice * expNumSlices;

	err = gpaAdjustTileMode(&pOut->tilemode, pIn->tilemode, expTileMode);
	if (err != GPA_ERR_OK) {
		return err;
	}

	pOut->depthalign = microTileThickness;

	return GPA_ERR_OK;
}

static GpaError DispatchComputeSurfaceInfo(
    GpaSurfaceInfo* out, const GpaTilingParams* params
) {
	const GnmArrayMode arrayMode = gpaGetArrayMode(params->tilemode);
	uint32_t bpp = params->bitsperfrag;
	uint32_t numSamples = params->numfragsperpixel;
	uint32_t numSlices = params->lineardepth;
	uint32_t mipLevel = params->miplevel;
	GpaSurfaceFlags flags = params->surfaceflags;

	uint32_t padDims = 0;

	// For macro tile mode, we should calculate default tiling
	// parameters
	GpaError err = gpaGetTileInfo(
	    &out->tileinfo, params->tilemode, bpp, numSamples,
	    params->mingpumode
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	if (flags.cube) {
		if (mipLevel == 0) {
			padDims = 2;
		}

		if (numSlices == 1) {
			// This is calculating one face, remove cube flag
			flags.cube = 0;
		}
	}

	switch (arrayMode) {
	case GNM_ARRAY_LINEAR_GENERAL:	// fall through
	case GNM_ARRAY_LINEAR_ALIGNED:
		err = ComputeSurfaceInfoLinear(params, out, padDims);
		break;

	case GNM_ARRAY_1D_TILED_THIN1:	// fall through
	case GNM_ARRAY_1D_TILED_THICK:
		err = ComputeSurfaceInfoMicroTiled(
		    params, out, padDims, arrayMode
		);
		break;

	case GNM_ARRAY_2D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_2D_TILED_THICK:	    // fall through
	case GNM_ARRAY_3D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_3D_TILED_THICK:	    // fall through
	case GNM_ARRAY_2D_TILED_XTHICK:	    // fall through
	case GNM_ARRAY_3D_TILED_XTHICK:	    // fall through
	case GNM_ARRAY_PRT_TILED_THIN1:	    // fall through
	case GNM_ARRAY_PRT_2D_TILED_THIN1:  // fall through
	case GNM_ARRAY_PRT_3D_TILED_THIN1:  // fall through
	case GNM_ARRAY_PRT_TILED_THICK:	    // fall through
	case GNM_ARRAY_PRT_2D_TILED_THICK:  // fall through
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		err = ComputeSurfaceInfoMacroTiled(
		    params, out, padDims, arrayMode
		);
		break;

	default:
		return GPA_ERR_INTERNAL_ERROR;
	}

	return err;
}

GpaError gpaComputeSurfaceInfo(
    GpaSurfaceInfo* out, const GpaTilingParams* params
) {
	if (!out || !params) {
		return GPA_ERR_INVALID_ARGS;
	}

	// We suggest client do sanity check but a check here is also good
	if (params->bitsperfrag > 128) {
		return GPA_ERR_INVALID_ARGS;
	}

	uint32_t bitsperelem = params->bitsperfrag;

	if (!IsPow2(bitsperelem) || bitsperelem < 1 || bitsperelem > 128) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmArrayMode arraymode = gpaGetArrayMode(params->tilemode);

	// Thick modes don't support multisample
	if (gpaGetMicroTileThickness(arraymode) > 1 &&
	    params->numfragsperpixel > 1) {
		return GPA_ERR_INVALID_ARGS;
	}

	// Do mipmap check first
	// If format is BCn, pre-pad dimension to power-of-two according to
	// HWL
	uint32_t linearwidth = 0;
	uint32_t linearheight = 0;
	ComputeMipLevel(&linearwidth, &linearheight, params);

	if (params->numfragsperpixel > 1 && params->miplevel != 0) {
		return GPA_ERR_INVALID_ARGS;
	}

	// Get compression/expansion factors and element mode (which
	// indicates compression/expansion
	uint32_t basePitch = params->basetiledpitch;
	uint32_t expandX = 1;
	uint32_t expandY = 1;
	/*if (params->isblockcompressed) {
		// Evergreen family workaround
		switch (bitsperelem) {
		case 1:
			expandX = 8;
			bitsperelem = 8;

			// Not BCn format we still keep old way (FMT_1? No
			// real test yet)
			linearwidth = (linearwidth + expandX - 1) / expandX;
			basePitch = (basePitch + expandX - 1) / expandX;
			break;
		case 4:
		case 8:
			expandX = 4;
			expandY = 4;
			bitsperelem *= 16;

			// For BCn we now pad it to POW2 at the
			// beginning so it is safe to divide by
			// 4 directly
			linearwidth = (linearwidth + expandX - 1) / expandX;
			linearheight = (linearheight + expandY - 1) / expandY;
			basePitch = (basePitch + expandX - 1) / expandX;
			break;
		case 16:
			return GPA_ERR_UNSUPPORTED;
		default:
			return GPA_ERR_INVALID_ARGS;
		}
	}*/

	// Mipmap including level 0 must be pow2 padded since either SI
	// hw expects so or it is required by CFX  for Hw Compatibility
	// between NI and SI. Otherwise it is only needed for mipLevel >
	// 0. Any h/w has different requirement should implement its own
	// virtual function

	uint32_t lineardepth = params->lineardepth;
	if (params->surfaceflags.pow2pad) {
		linearwidth = NextPow2(linearwidth);
		linearheight = NextPow2(linearheight);
		lineardepth = NextPow2(lineardepth);
	} else if (params->miplevel > 0) {
		linearwidth = NextPow2(linearwidth);
		linearheight = NextPow2(linearheight);

		if (!params->surfaceflags.cube) {
			lineardepth = NextPow2(lineardepth);
		}

		// for cubemap, we keep its value at first
	}

	GpaTilingParams tpcopy = *params;
	tpcopy.linearwidth = linearwidth;
	tpcopy.linearheight = linearheight;
	tpcopy.lineardepth = lineardepth;
	tpcopy.basetiledpitch = basePitch;

	GpaError err = DispatchComputeSurfaceInfo(out, &tpcopy);
	if (err != GPA_ERR_OK) {
		return err;
	}

	// ElemLib::RestoreSurfaceInfo
	out->height *= expandY;
	out->pitchalign *= expandX;
	out->heightalign *= expandY;
	if (params->surfaceflags.pow2pad) {
		out->pitch = NextPow2(out->pitch);
		out->height = NextPow2(out->height);
		out->depth = NextPow2(out->depth);
	}

	return GPA_ERR_OK;
}

static inline void HwlComputeTileDataWidthAndHeightLinear(
    uint32_t* pMacroWidth,    ///< [out] macro tile width
    uint32_t* pMacroHeight,   ///< [out] macro tile height
    GnmPipeConfig pipeConfig  ///< [in] pipe configuration
) {
	uint32_t numTiles = 0;

	switch (pipeConfig) {
	case GNM_ADDR_SURF_P16_32x32_8x16:
	case GNM_ADDR_SURF_P8_32x32_8x16:
		numTiles = 8;
		break;
	default:
		numTiles = 4;
		break;
	}

	*pMacroWidth = numTiles * MicroTileWidth;
	*pMacroHeight = numTiles * MicroTileHeight;
}

static inline void ComputeTileDataWidthAndHeight(
    uint32_t bpp,	       ///< [in] bits per pixel
    uint32_t cacheBits,	       ///< [in] bits of cache
    GnmPipeConfig pipeConfig,  ///< [in] pipe configuration
    uint32_t* pMacroWidth,     ///< [out] macro tile width
    uint32_t* pMacroHeight     ///< [out] macro tile height
) {
	uint32_t height = 1;
	uint32_t width = cacheBits / bpp;
	uint32_t pipes = gpaGetPipeCount(pipeConfig);

	// Double height until the macro-tile is close to square
	// Height can only be doubled if width is even

	while ((width > height * 2 * pipes) && !(width & 1)) {
		width /= 2;
		height *= 2;
	}

	*pMacroWidth = 8 * width;
	*pMacroHeight = 8 * height * pipes;

	// Note: The above iterative comptuation is equivalent to the
	// following
	//
	// int log2_height =
	// ((log2(cacheBits)-log2(bpp)-log2(pipes))/2); int macroHeight
	// = pow2( 3+log2(pipes)+log2_height );
}

GpaError gpaComputeHtileInfo(
    GpaHtileInfo* outinfo, const GpaHtileParams* params
) {
	if (!outinfo || !params) {
		return GPA_ERR_INVALID_ARGS;
	}

	const uint32_t banks = 1 << params->banks;
	const uint32_t pipes = gpaGetPipeCount(params->pipeconfig);

	if (params->flags.tccompatible) {
		const uint32_t sliceSize =
		    params->pitch * params->height * 4 / (8 * 8);
		const uint32_t align = pipes * banks * PIPE_INTERLEAVE_BYTES;

		outinfo->macrowidth =
		    8 * 512 /
		    params->bpp;  // Align width to 512-bit memory accesses
		outinfo->macroheight =
		    8 * pipes;	// Align height to number of pipes

		outinfo->htilebytes = sliceSize;

		outinfo->pitch = params->pitch;
		outinfo->height = params->height;
		outinfo->basealign = align;
		outinfo->macrowidth = 0;
		outinfo->macroheight = 0;
		outinfo->bpp = 32;
	} else {
		uint32_t macroWidth = 0;
		uint32_t macroHeight = 0;
		uint32_t baseAlign = 0;
		uint64_t surfBytes = 0;
		uint64_t sliceBytes = 0;

		const uint32_t numSlices = umax(1u, params->numslices);

		const uint32_t bpp = 32;
		const uint32_t cacheBits = HtileCacheBits;

		const bool islinear = gpaIsLinear(params->arraymode);

		if (islinear) {
			HwlComputeTileDataWidthAndHeightLinear(
			    &macroWidth, &macroHeight, params->pipeconfig
			);
		} else {
			ComputeTileDataWidthAndHeight(
			    bpp, cacheBits, params->pipeconfig, &macroWidth,
			    &macroHeight
			);
		}

		outinfo->pitch = PowTwoAlign32(params->pitch, macroWidth);
		outinfo->height = PowTwoAlign32(params->height, macroHeight);

		baseAlign = PIPE_INTERLEAVE_BYTES * pipes;
		if (params->flags.tccompatible) {
			baseAlign *= banks;
		}

		outinfo->basealign = baseAlign;

		outinfo->macrowidth = macroWidth;
		outinfo->macroheight = macroHeight;

		const uint64_t HtileCacheLineSize =
		    BitsToBytes64(HtileCacheBits);

		sliceBytes = BitsToBytes64(
		    (uint64_t)outinfo->pitch * outinfo->height * bpp / 64
		);

		// Align the surfSize to htilecachelinesize * pipes at
		// last
		surfBytes = sliceBytes * numSlices;
		surfBytes =
		    PowTwoAlign32(surfBytes, HtileCacheLineSize * pipes);

		outinfo->htilebytes = surfBytes;
		outinfo->slicebytes = sliceBytes;

		outinfo->bpp = params->bpp;
	}

	return GPA_ERR_OK;
}

static inline uint64_t ComputeCmaskBytes(
    uint32_t pitch,	///< [in] pitch
    uint32_t height,	///< [in] height
    uint32_t numSlices	///< [in] number of slices
) {
	return BitsToBytes64(
		   (uint64_t)pitch * height * numSlices * CmaskElemBits
	       ) /
	       MicroTilePixels;
}

static inline uint32_t ComputeCmaskBaseAlign(
    const GpaTileInfo* pTileInfo,  ///< [in] Tile info
    bool tcCompatible		   ///< [in] should be shader readable
) {
	uint32_t baseAlign =
	    PIPE_INTERLEAVE_BYTES * gpaGetPipeCount(pTileInfo->pipeconfig);

	if (tcCompatible) {
		baseAlign *= (1 << pTileInfo->banks);
	}

	return baseAlign;
}

GpaError gpaComputeCmaskInfo(
    GpaCmaskInfo* outinfo, const GpaCmaskParams* params
) {
	if (!outinfo || !params) {
		return GPA_ERR_INVALID_ARGS;
	}

	GpaTileInfo tileinfo = {0};
	GpaError err = gpaGetTileInfo(
	    &tileinfo, params->tilemode, params->bpp, params->numfrags,
	    params->mingpumode
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	uint32_t macroWidth = 0;
	uint32_t macroHeight = 0;
	uint32_t baseAlign = 0;
	uint64_t surfBytes = 0;
	uint64_t sliceBytes = 0;

	uint32_t numSlices = umax(1u, params->numslices);

	const uint32_t bpp = CmaskElemBits;
	const uint32_t cacheBits = CmaskCacheBits;
	const GnmArrayMode arrayMode = gpaGetArrayMode(params->tilemode);
	const bool islinear = gpaIsLinear(arrayMode);

	if (islinear) {
		HwlComputeTileDataWidthAndHeightLinear(
		    &macroWidth, &macroHeight, tileinfo.pipeconfig
		);
	} else {
		ComputeTileDataWidthAndHeight(
		    bpp, cacheBits, tileinfo.pipeconfig, &macroWidth,
		    &macroHeight
		);
	}

	outinfo->pitch = (params->pitch + macroWidth - 1) & ~(macroWidth - 1);
	outinfo->height =
	    (params->height + macroHeight - 1) & ~(macroHeight - 1);

	sliceBytes = ComputeCmaskBytes(outinfo->pitch, outinfo->height, 1);

	baseAlign =
	    ComputeCmaskBaseAlign(&tileinfo, params->flags.tccompatible);

	while (sliceBytes % baseAlign) {
		outinfo->height += macroHeight;

		sliceBytes =
		    ComputeCmaskBytes(outinfo->pitch, outinfo->height, 1);
	}

	surfBytes = sliceBytes * numSlices;

	outinfo->cmaskbytes = surfBytes;

	//
	// Use SafeAssign since they are optional
	//
	outinfo->macrowidth = macroWidth;
	outinfo->macroheight = macroHeight;
	outinfo->basealign = baseAlign;
	outinfo->slicebytes = sliceBytes;

	uint32_t slice = (outinfo->pitch) * (outinfo->height);
	uint32_t blockMax = slice / 128 / 128 - 1;

	uint32_t maxBlockMax = 0x3FFF;	// 14 bits, 0n16383

	if (blockMax > maxBlockMax) {
		blockMax = maxBlockMax;
		return GPA_ERR_INVALID_ARGS;
	}

	outinfo->blockmax = blockMax;

	return GPA_ERR_OK;
}

GpaError gpaComputeFmaskInfo(
    GpaFmaskInfo* outinfo, const GpaFmaskParams* params
) {
	if (!outinfo || !params || params->numfrags <= 1) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GpaTilingParams tp = {
	    .tilemode = params->tilemode,
	    .mingpumode = params->mingpumode,

	    .linearwidth = params->pitch,
	    .linearheight = params->height,
	    .lineardepth = params->numslices,
	    .numfragsperpixel = params->numfrags,
	    .basetiledpitch = 0,

	    .miplevel = 0,
	    .arrayslice = 0,
	    .surfaceflags =
		{
		    .fmask = 1,
		    .texcompatible = params->mingpumode == GNM_GPU_NEO,
		},
	    .bitsperfrag = params->bpp,
	    .isblockcompressed = params->isblockcompressed,
	};

	GpaSurfaceInfo surfinfo = {0};
	GpaError err = gpaComputeSurfaceInfo(&surfinfo, &tp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	*outinfo = (GpaFmaskInfo){
	    .pitch = surfinfo.pitch,
	    .height = surfinfo.height,
	    .basealign = surfinfo.basealign,
	    .pitchalign = surfinfo.pitchalign,
	    .heightalign = surfinfo.heightalign,
	    .bpp = surfinfo.bitsperelem,
	    .fmaskbytes = surfinfo.surfacesize * params->numslices,
	    .slicebytes = surfinfo.surfacesize,
	};
	return GPA_ERR_OK;
}

GpaError gpaInitSurfaceContext(
    GpaSurfaceContext* ctx, size_t surfsize, const GpaTilingParams* tp
) {
	GpaSurfaceInfo surfinfo = {0};
	GpaError err = gpaComputeSurfaceInfo(&surfinfo, tp);
	if (err != GPA_ERR_OK) {
		return err;
	}

	if (surfinfo.surfacesize > surfsize) {
		return GPA_ERR_OVERFLOW;
	}

	GpaTileInfo tileinfo = {0};
	err = gpaGetTileInfo(
	    &tileinfo, tp->tilemode, tp->bitsperfrag, tp->numfragsperpixel,
	    tp->mingpumode
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	*ctx = (GpaSurfaceContext){
	    .mingpumode = tp->mingpumode,
	    .tilemode = tp->tilemode,
	    .tileinfo = tileinfo,

	    .linearwidth = tp->linearwidth,
	    .linearheight = tp->linearheight,
	    .lineardepth = tp->lineardepth,
	    .paddedwidth = surfinfo.pitch,
	    .paddedheight = surfinfo.height,
	    .paddeddepth = surfinfo.depth,

	    .bitsperelement = tp->bitsperfrag,
	    .numfragsperpixel = tp->numfragsperpixel,

	    // TODO: calc swizzle?
	    .bankswizzlemask = 0,
	    .pipeswizzlemask = 0,
	};

	// TODO: why is this here?
	// BC7 tests fail is this isn't here,
	// but shouldn't this be handled by something else already?
	if (tp->isblockcompressed) {
		switch (tp->bitsperfrag) {
		case 1:
			ctx->bitsperelement *= 8;
			ctx->linearwidth = umax((ctx->linearwidth + 7) / 8, 1);
			ctx->paddedwidth = umax((ctx->paddedwidth + 7) / 8, 1);
			break;
		case 4:
		case 8:
			ctx->bitsperelement *= 16;
			ctx->linearwidth = umax((ctx->linearwidth + 3) / 4, 1);
			ctx->linearheight =
			    umax((ctx->linearheight + 3) / 4, 1);
			ctx->paddedwidth = umax((ctx->paddedwidth + 3) / 4, 1);
			ctx->paddedheight =
			    umax((ctx->paddedheight + 3) / 4, 1);
			break;
		case 16:
		default:
			return GPA_ERR_UNSUPPORTED;
		}
	}

	return GPA_ERR_OK;
}

static GpaError ComputePixelIndexWithinMicroTile(
    uint32_t* outIndex,
    uint32_t x,		     ///< [in] x coord
    uint32_t y,		     ///< [in] y coord
    uint32_t z,		     ///< [in] slice/depth index
    uint32_t bpp,	     ///< [in] bits per pixel
    GnmArrayMode arrayMode,  ///< [in] tile mode
    GnmMicroTileMode
	microTileType  ///< [in] pixel order in display/non-display mode
) {
	uint32_t pixelBit0 = 0;
	uint32_t pixelBit1 = 0;
	uint32_t pixelBit2 = 0;
	uint32_t pixelBit3 = 0;
	uint32_t pixelBit4 = 0;
	uint32_t pixelBit5 = 0;
	uint32_t pixelBit6 = 0;
	uint32_t pixelBit7 = 0;
	uint32_t pixelBit8 = 0;

	const uint32_t x0 = (x >> 0) & 1;
	const uint32_t x1 = (x >> 1) & 1;
	const uint32_t x2 = (x >> 2) & 1;
	const uint32_t y0 = (y >> 0) & 1;
	const uint32_t y1 = (y >> 1) & 1;
	const uint32_t y2 = (y >> 2) & 1;
	const uint32_t z0 = (z >> 0) & 1;
	const uint32_t z1 = (z >> 1) & 1;
	const uint32_t z2 = (z >> 2) & 1;

	const uint32_t thickness = gpaGetMicroTileThickness(arrayMode);

	// Compute the pixel number within the micro tile.

	if (microTileType != GNM_SURF_THICK_MICRO_TILING) {
		if (microTileType == GNM_SURF_DISPLAY_MICRO_TILING) {
			switch (bpp) {
			case 8:
				pixelBit0 = x0;
				pixelBit1 = x1;
				pixelBit2 = x2;
				pixelBit3 = y1;
				pixelBit4 = y0;
				pixelBit5 = y2;
				break;
			case 16:
				pixelBit0 = x0;
				pixelBit1 = x1;
				pixelBit2 = x2;
				pixelBit3 = y0;
				pixelBit4 = y1;
				pixelBit5 = y2;
				break;
			case 32:
				pixelBit0 = x0;
				pixelBit1 = x1;
				pixelBit2 = y0;
				pixelBit3 = x2;
				pixelBit4 = y1;
				pixelBit5 = y2;
				break;
			case 64:
				pixelBit0 = x0;
				pixelBit1 = y0;
				pixelBit2 = x1;
				pixelBit3 = x2;
				pixelBit4 = y1;
				pixelBit5 = y2;
				break;
			case 128:
				pixelBit0 = y0;
				pixelBit1 = x0;
				pixelBit2 = x1;
				pixelBit3 = x2;
				pixelBit4 = y1;
				pixelBit5 = y2;
				break;
			default:
				return GPA_ERR_INTERNAL_ERROR;
			}
		} else if (microTileType != GNM_SURF_DISPLAY_MICRO_TILING || microTileType == GNM_SURF_DEPTH_MICRO_TILING) {
			pixelBit0 = x0;
			pixelBit1 = y0;
			pixelBit2 = x1;
			pixelBit3 = y1;
			pixelBit4 = x2;
			pixelBit5 = y2;
		} else if (microTileType == GNM_SURF_ROTATED_MICRO_TILING) {
			if (thickness != 1) {
				return GPA_ERR_INTERNAL_ERROR;
			}

			switch (bpp) {
			case 8:
				pixelBit0 = y0;
				pixelBit1 = y1;
				pixelBit2 = y2;
				pixelBit3 = x1;
				pixelBit4 = x0;
				pixelBit5 = x2;
				break;
			case 16:
				pixelBit0 = y0;
				pixelBit1 = y1;
				pixelBit2 = y2;
				pixelBit3 = x0;
				pixelBit4 = x1;
				pixelBit5 = x2;
				break;
			case 32:
				pixelBit0 = y0;
				pixelBit1 = y1;
				pixelBit2 = x0;
				pixelBit3 = y2;
				pixelBit4 = x1;
				pixelBit5 = x2;
				break;
			case 64:
				pixelBit0 = y0;
				pixelBit1 = x0;
				pixelBit2 = y1;
				pixelBit3 = x1;
				pixelBit4 = x2;
				pixelBit5 = y2;
				break;
			default:
				return GPA_ERR_INTERNAL_ERROR;
			}
		}

		if (thickness > 1) {
			pixelBit6 = z0;
			pixelBit7 = z1;
		}
	} else	// ADDR_THICK
	{
		if (thickness <= 1) {
			return GPA_ERR_INTERNAL_ERROR;
		}

		switch (bpp) {
		case 8:
		case 16:
			pixelBit0 = x0;
			pixelBit1 = y0;
			pixelBit2 = x1;
			pixelBit3 = y1;
			pixelBit4 = z0;
			pixelBit5 = z1;
			break;
		case 32:
			pixelBit0 = x0;
			pixelBit1 = y0;
			pixelBit2 = x1;
			pixelBit3 = z0;
			pixelBit4 = y1;
			pixelBit5 = z1;
			break;
		case 64:
		case 128:
			pixelBit0 = x0;
			pixelBit1 = y0;
			pixelBit2 = z0;
			pixelBit3 = x1;
			pixelBit4 = y1;
			pixelBit5 = z1;
			break;
		default:
			return GPA_ERR_INTERNAL_ERROR;
		}

		pixelBit6 = x2;
		pixelBit7 = y2;
	}

	if (thickness == 8) {
		pixelBit8 = z2;
	}

	*outIndex =
	    ((pixelBit0) | (pixelBit1 << 1) | (pixelBit2 << 2) |
	     (pixelBit3 << 3) | (pixelBit4 << 4) | (pixelBit5 << 5) |
	     (pixelBit6 << 6) | (pixelBit7 << 7) | (pixelBit8 << 8));
	return GPA_ERR_OK;
}

uint64_t gpaComputeSurfaceAddrFromCoordLinear(
    uint32_t x,		    ///< [in] x coord
    uint32_t y,		    ///< [in] y coord
    uint32_t slice,	    ///< [in] slice/depth index
    uint32_t sample,	    ///< [in] sample index
    uint32_t bpp,	    ///< [in] bits per pixel
    uint32_t pitch,	    ///< [in] pitch
    uint32_t height,	    ///< [in] height
    uint32_t numSlices,	    ///< [in] number of slices
    uint32_t* pBitPosition  ///< [out] bit position inside a byte
) {
	const uint64_t sliceSize = (uint64_t)pitch * height;

	uint64_t sliceOffset = (slice + sample * numSlices) * sliceSize;
	uint64_t rowOffset = (uint64_t)y * pitch;
	uint64_t pixOffset = x;

	uint64_t addr = (sliceOffset + rowOffset + pixOffset) * bpp;

	if (pBitPosition) {
		*pBitPosition = (uint32_t)(addr % 8);
	}
	addr /= 8;

	return addr;
}

static GpaError ComputeSurfaceAddrFromCoordMicroTiled(
    uint32_t x,			     ///< [in] x coordinate
    uint32_t y,			     ///< [in] y coordinate
    uint32_t slice,		     ///< [in] slice index
    uint32_t sample,		     ///< [in] sample index
    uint32_t bpp,		     ///< [in] bits per pixel
    uint32_t pitch,		     ///< [in] pitch, in pixels
    uint32_t height,		     ///< [in] height, in pixels
    uint32_t numSamples,	     ///< [in] number of samples
    GnmArrayMode arrayMode,	     ///< [in] tile mode
    GnmMicroTileMode microTileType,  ///< [in] micro tiling type
    bool isDepthSampleOrder,  ///< [in] TRUE if depth sample ordering is used
    uint64_t* outAddr,	      ///< [out] byte position
    uint32_t* pBitPosition    ///< [out] bit position, e.g. FMT_1 will use this
) {
	uint32_t microTileBytes;
	uint64_t sliceBytes;
	uint32_t microTilesPerRow;
	uint32_t microTileIndexX;
	uint32_t microTileIndexY;
	uint32_t microTileIndexZ;
	uint64_t sliceOffset;
	uint64_t microTileOffset;
	uint32_t sampleOffset;
	uint32_t pixelIndex;
	uint32_t pixelOffset;

	const uint32_t microTileThickness = gpaGetMicroTileThickness(arrayMode);

	//
	// Compute the micro tile size.
	//
	microTileBytes = BitsToBytes32(
	    MicroTilePixels * microTileThickness * bpp * numSamples
	);

	//
	// Compute the slice size.
	//
	sliceBytes = BitsToBytes64(
	    (uint64_t)pitch * height * microTileThickness * bpp * numSamples
	);

	//
	// Compute the number of micro tiles per row.
	//
	microTilesPerRow = pitch / MicroTileWidth;

	//
	// Compute the micro tile index.
	//
	microTileIndexX = x / MicroTileWidth;
	microTileIndexY = y / MicroTileHeight;
	microTileIndexZ = slice / microTileThickness;

	//
	// Compute the slice offset.
	//
	sliceOffset = (uint64_t)microTileIndexZ * sliceBytes;

	//
	// Compute the offset to the micro tile containing the specified
	// coordinate.
	//
	microTileOffset =
	    ((uint64_t)microTileIndexY * microTilesPerRow + microTileIndexX) *
	    microTileBytes;

	//
	// Compute the pixel index within the micro tile.
	//
	GpaError err = ComputePixelIndexWithinMicroTile(
	    &pixelIndex, x, y, slice, bpp, arrayMode, microTileType
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	// Compute the sample offset.
	//
	if (isDepthSampleOrder) {
		//
		// For depth surfaces, samples are stored contiguously
		// for each element, so the sample offset is the sample
		// number times the element size.
		//
		sampleOffset = sample * bpp;
		pixelOffset = pixelIndex * bpp * numSamples;
	} else {
		//
		// For color surfaces, all elements for a particular
		// sample are stored contiguously, so the sample offset
		// is the sample number times the micro tile size
		// divided yBit the number of samples.
		//
		sampleOffset = sample * (microTileBytes * 8 / numSamples);
		pixelOffset = pixelIndex * bpp;
	}

	//
	// Compute the bit position of the pixel.  Each element is
	// stored with one bit per sample.
	//

	uint32_t elemOffset = sampleOffset + pixelOffset;

	*pBitPosition = elemOffset % 8;
	elemOffset /= 8;

	//
	// Combine the slice offset, micro tile offset, sample offset,
	// and pixel offsets.
	//
	*outAddr = sliceOffset + microTileOffset + elemOffset;

	return GPA_ERR_OK;
}

static uint32_t ComputePipeFromCoord(
    uint32_t x,			   ///< [in] x coordinate
    uint32_t y,			   ///< [in] y coordinate
    uint32_t slice,		   ///< [in] slice index
    GnmArrayMode arrayMode,	   ///< [in] tile mode
    uint32_t pipeSwizzle,	   ///< [in] pipe swizzle
    const GpaTileInfo* pTileInfo,  ///< [in] Tile info
    uint32_t* outCoord		   ///< [out] resulting coordinate
) {
	uint32_t pipe;
	uint32_t pipeBit0 = 0;
	uint32_t pipeBit1 = 0;
	uint32_t pipeBit2 = 0;
	uint32_t pipeBit3 = 0;
	uint32_t sliceRotation;
	uint32_t numPipes = 0;

	const uint32_t tx = x / MicroTileWidth;
	const uint32_t ty = y / MicroTileHeight;
	const uint32_t x3 = (tx >> 0) & 1;
	const uint32_t x4 = (tx >> 1) & 1;
	const uint32_t x5 = (tx >> 2) & 1;
	const uint32_t x6 = (tx >> 3) & 1;
	const uint32_t y3 = (ty >> 0) & 1;
	const uint32_t y4 = (ty >> 1) & 1;
	const uint32_t y5 = (ty >> 2) & 1;
	const uint32_t y6 = (ty >> 3) & 1;

	switch (pTileInfo->pipeconfig) {
	case GNM_ADDR_SURF_P2:
		pipeBit0 = x3 ^ y3;
		numPipes = 2;
		break;
	case GNM_ADDR_SURF_P8_32x32_8x16:
		pipeBit0 = x4 ^ y3 ^ x5;
		pipeBit1 = x3 ^ y4;
		pipeBit2 = x5 ^ y5;
		numPipes = 8;
		break;
	case GNM_ADDR_SURF_P8_32x32_16x16:
		pipeBit0 = x3 ^ y3 ^ x4;
		pipeBit1 = x4 ^ y4;
		pipeBit2 = x5 ^ y5;
		numPipes = 8;
		break;
	case GNM_ADDR_SURF_P16_32x32_8x16:
		pipeBit0 = x4 ^ y3;
		pipeBit1 = x3 ^ y4;
		pipeBit2 = x5 ^ y6;
		pipeBit3 = x6 ^ y5;
		numPipes = 16;
		break;
	default:
		return GPA_ERR_UNSUPPORTED;
	}

	pipe = pipeBit0 | (pipeBit1 << 1) | (pipeBit2 << 2) | (pipeBit3 << 3);

	const uint32_t microTileThickness = gpaGetMicroTileThickness(arrayMode);

	//
	// Apply pipe rotation for the slice.
	//
	switch (arrayMode) {
	case GNM_ARRAY_3D_TILED_THIN1:	// fall through thin
	case GNM_ARRAY_3D_TILED_THICK:	// fall through thick
	case GNM_ARRAY_3D_TILED_XTHICK:
		sliceRotation = umax(1, (int32_t)(numPipes / 2) - 1) *
				(slice / microTileThickness);
		break;
	default:
		sliceRotation = 0;
		break;
	}
	pipeSwizzle += sliceRotation;
	pipeSwizzle &= (numPipes - 1);

	*outCoord = pipe ^ pipeSwizzle;
	return GPA_ERR_OK;
}

static GpaError ComputeBankFromCoord(
    uint32_t x,			   ///< [in] x coordinate
    uint32_t y,			   ///< [in] y coordinate
    uint32_t slice,		   ///< [in] slice index
    GnmArrayMode arrayMode,	   ///< [in] tile mode
    uint32_t bankSwizzle,	   ///< [in] bank swizzle
    uint32_t tileSplitSlice,	   ///< [in] If the size of the pixel offset is
				   ///< larger than the
				   ///  tile split size, then the pixel will be
				   ///  moved to a separate slice. This value
				   ///  equals pixelOffset / tileSplitBytes in
				   ///  this case. Otherwise this is 0.
    const GpaTileInfo* pTileInfo,  ///< [in] tile info
    uint32_t* outCoord		   ///< [out] resulting coord
) {
	const uint32_t pipes = gpaGetPipeCount(pTileInfo->pipeconfig);
	uint32_t bankBit0 = 0;
	uint32_t bankBit1 = 0;
	uint32_t bankBit2 = 0;
	uint32_t bankBit3 = 0;
	uint32_t sliceRotation;
	uint32_t tileSplitRotation;
	uint32_t bank;
	const uint32_t numBanks = 2 << pTileInfo->banks;
	const uint32_t bankWidth = (1 << pTileInfo->bankwidth);
	const uint32_t bankHeight = (1 << pTileInfo->bankheight);

	const uint32_t tx = x / MicroTileWidth / (bankWidth * pipes);
	const uint32_t ty = y / MicroTileHeight / bankHeight;

	const uint32_t x3 = (tx >> 0) & 1;
	const uint32_t x4 = (tx >> 1) & 1;
	const uint32_t x5 = (tx >> 2) & 1;
	const uint32_t x6 = (tx >> 3) & 1;
	const uint32_t y3 = (ty >> 0) & 1;
	const uint32_t y4 = (ty >> 1) & 1;
	const uint32_t y5 = (ty >> 2) & 1;
	const uint32_t y6 = (ty >> 3) & 1;

	switch (numBanks) {
	case 16:
		bankBit0 = x3 ^ y6;
		bankBit1 = x4 ^ y5 ^ y6;
		bankBit2 = x5 ^ y4;
		bankBit3 = x6 ^ y3;
		break;
	case 8:
		bankBit0 = x3 ^ y5;
		bankBit1 = x4 ^ y4 ^ y5;
		bankBit2 = x5 ^ y3;
		break;
	case 4:
		bankBit0 = x3 ^ y4;
		bankBit1 = x4 ^ y3;
		break;
	case 2:
		bankBit0 = x3 ^ y3;
		break;
	default:
		return GPA_ERR_UNSUPPORTED;
	}

	bank = bankBit0 | (bankBit1 << 1) | (bankBit2 << 2) | (bankBit3 << 3);

	// Bits2Number(4, bankBit3, bankBit2, bankBit1, bankBit0);

	// bank = HwlPreAdjustBank((x / MicroTileWidth), bank, pTileInfo);
	//
	//  Compute bank rotation for the slice.
	//
	const uint32_t microTileThickness = gpaGetMicroTileThickness(arrayMode);

	switch (arrayMode) {
	case GNM_ARRAY_2D_TILED_THIN1:	// fall through
	case GNM_ARRAY_2D_TILED_THICK:	// fall through
	case GNM_ARRAY_2D_TILED_XTHICK:
		sliceRotation =
		    ((numBanks / 2) - 1) * (slice / microTileThickness);
		break;
	case GNM_ARRAY_3D_TILED_THIN1:	// fall through
	case GNM_ARRAY_3D_TILED_THICK:	// fall through
	case GNM_ARRAY_3D_TILED_XTHICK:
		sliceRotation = umax(1u, (pipes / 2) - 1) *
				(slice / microTileThickness) / pipes;
		break;
	default:
		sliceRotation = 0;
		break;
	}

	//
	// Compute bank rotation for the tile split slice.
	//
	// The sample slice will be non-zero if samples must be split
	// across multiple slices. This situation arises when the micro
	// tile size multiplied yBit the number of samples exceeds the
	// split size (set in GB_ADDR_CONFIG).
	//
	switch (arrayMode) {
	case GNM_ARRAY_2D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_3D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_PRT_2D_TILED_THIN1:  // fall through
	case GNM_ARRAY_PRT_3D_TILED_THIN1:  // fall through
		tileSplitRotation = ((numBanks / 2) + 1) * tileSplitSlice;
		break;
	default:
		tileSplitRotation = 0;
		break;
	}

	//
	// Apply bank rotation for the slice and tile split slice.
	//
	bank ^= bankSwizzle + sliceRotation;
	bank ^= tileSplitRotation;

	bank &= (numBanks - 1);

	*outCoord = bank;
	return GPA_ERR_OK;
}

static GpaError ComputeSurfaceAddrFromCoordMacroTiled(
    uint32_t x,			     ///< [in] x coordinate
    uint32_t y,			     ///< [in] y coordinate
    uint32_t slice,		     ///< [in] slice index
    uint32_t sample,		     ///< [in] sample index
    uint32_t bpp,		     ///< [in] bits per pixel
    uint32_t pitch,		     ///< [in] surface pitch, in pixels
    uint32_t height,		     ///< [in] surface height, in pixels
    uint32_t numSamples,	     ///< [in] number of samples
    GnmArrayMode arrayMode,	     ///< [in] tile mode
    GnmMicroTileMode microTileType,  ///< [in] micro tiling type
    bool isDepthSampleOrder,  ///< [in] TRUE if it depth sample ordering is used
    uint32_t pipeSwizzle,     ///< [in] pipe swizzle
    uint32_t bankSwizzle,     ///< [in] bank swizzle
    const GpaTileInfo* pTileInfo,  ///< [in] bank structure  **All fields to be
				   ///< valid on entry**
    uint64_t* pBytePosition,	   ///< [out] byte position
    uint32_t* pBitPosition  ///< [out] bit position, e.g. FMT_1 will use this
) {
	uint64_t addr;

	uint32_t microTileBytes;
	uint32_t microTileBits;
	uint32_t sampleOffset;
	uint32_t pixelIndex;
	uint32_t pixelOffset;
	uint32_t elementOffset;
	uint32_t tileSplitSlice;
	uint32_t pipe;
	uint32_t bank;
	uint64_t sliceBytes;
	uint64_t sliceOffset;
	uint32_t macroTilePitch;
	uint32_t macroTileHeight;
	uint32_t macroTilesPerRow;
	uint32_t macroTilesPerSlice;
	uint64_t macroTileBytes;
	uint32_t macroTileIndexX;
	uint32_t macroTileIndexY;
	uint64_t macroTileOffset;
	uint64_t totalOffset;
	uint64_t pipeInterleaveMask;
	uint64_t bankInterleaveMask;
	uint64_t pipeInterleaveOffset;
	uint32_t bankInterleaveOffset;
	uint64_t offset;
	uint32_t tileRowIndex;
	uint32_t tileColumnIndex;
	uint32_t tileIndex;
	uint32_t tileOffset;

	uint32_t microTileThickness = gpaGetMicroTileThickness(arrayMode);

	const uint32_t banks = 2 << pTileInfo->banks;
	const uint32_t bankWidth = (1 << pTileInfo->bankwidth);
	const uint32_t bankHeight = (1 << pTileInfo->bankheight);
	const uint32_t macroAspectRatio = (1 << pTileInfo->macroaspectratio);
	const uint32_t tileSplitBytes =
	    GetTileSplitBytes(pTileInfo->tilesplit, bpp, microTileThickness);

	//
	// Compute the number of group, pipe, and bank bits.
	//
	uint32_t numPipes = gpaGetPipeCount(pTileInfo->pipeconfig);
	uint32_t numPipeInterleaveBits = log2(PIPE_INTERLEAVE_BYTES);
	uint32_t numPipeBits = log2(numPipes);
	uint32_t numBankInterleaveBits = log2(BANK_INTERLEAVE);
	uint32_t numBankBits = log2(banks);

	//
	// Compute the micro tile size.
	//
	microTileBits = MicroTilePixels * microTileThickness * bpp * numSamples;

	microTileBytes = microTileBits / 8;
	//
	// Compute the pixel index within the micro tile.
	//
	GpaError err = ComputePixelIndexWithinMicroTile(
	    &pixelIndex, x, y, slice, bpp, arrayMode, microTileType
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	//
	// Compute the sample offset and pixel offset.
	//
	if (isDepthSampleOrder) {
		//
		// For depth surfaces, samples are stored contiguously
		// for each element, so the sample offset is the sample
		// number times the element size.
		//
		sampleOffset = sample * bpp;
		pixelOffset = pixelIndex * bpp * numSamples;
	} else {
		//
		// For color surfaces, all elements for a particular
		// sample are stored contiguously, so the sample offset
		// is the sample number times the micro tile size
		// divided yBit the number of samples.
		//
		sampleOffset = sample * (microTileBits / numSamples);
		pixelOffset = pixelIndex * bpp;
	}

	//
	// Compute the element offset.
	//
	elementOffset = pixelOffset + sampleOffset;

	*pBitPosition = (uint32_t)(elementOffset % 8);

	elementOffset /= 8;  // bit-to-byte

	//
	// Determine if tiles need to be split across slices.
	//
	// If the size of the micro tile is larger than the tile split
	// size, then the tile will be split across multiple slices.
	//
	uint32_t slicesPerTile = 1;

	if ((microTileBytes > tileSplitBytes) &&
	    (microTileThickness == 1)) {  // don't support for thick mode

		//
		// Compute the number of slices per tile.
		//
		slicesPerTile = microTileBytes / tileSplitBytes;

		//
		// Compute the tile split slice number for use in
		// rotating the bank.
		//
		tileSplitSlice = elementOffset / tileSplitBytes;

		//
		// Adjust the element offset to account for the portion
		// of the tile that is being moved to a new slice..
		//
		elementOffset %= tileSplitBytes;

		//
		// Adjust the microTileBytes size to tileSplitBytes size
		// since a new slice..
		//
		microTileBytes = tileSplitBytes;
	} else {
		tileSplitSlice = 0;
	}

	//
	// Compute macro tile pitch and height.
	//
	macroTilePitch =
	    (MicroTileWidth * bankWidth * numPipes) * macroAspectRatio;
	macroTileHeight =
	    (MicroTileHeight * bankHeight * banks) / macroAspectRatio;

	//
	// Compute the number of bytes per macro tile. Note: bytes of
	// the same bank/pipe actually
	//
	macroTileBytes =
	    (uint64_t)microTileBytes * (macroTilePitch / MicroTileWidth) *
	    (macroTileHeight / MicroTileHeight) / (numPipes * banks);

	//
	// Compute the number of macro tiles per row.
	//
	macroTilesPerRow = pitch / macroTilePitch;

	//
	// Compute the offset to the macro tile containing the specified
	// coordinate.
	//
	macroTileIndexX = x / macroTilePitch;
	macroTileIndexY = y / macroTileHeight;
	macroTileOffset =
	    ((macroTileIndexY * macroTilesPerRow) + macroTileIndexX) *
	    macroTileBytes;

	//
	// Compute the number of macro tiles per slice.
	//
	macroTilesPerSlice = macroTilesPerRow * (height / macroTileHeight);

	//
	// Compute the slice size.
	//
	sliceBytes = macroTilesPerSlice * macroTileBytes;

	//
	// Compute the slice offset.
	//
	sliceOffset =
	    sliceBytes *
	    (tileSplitSlice + slicesPerTile * (slice / microTileThickness));

	//
	// Compute tile offest
	//
	tileRowIndex = (y / MicroTileHeight) % bankHeight;
	tileColumnIndex = ((x / MicroTileWidth) / numPipes) % bankWidth;
	tileIndex = (tileRowIndex * bankWidth) + tileColumnIndex;
	tileOffset = tileIndex * microTileBytes;

	//
	// Combine the slice offset and macro tile offset with the pixel
	// and sample offsets, accounting for the pipe and bank bits in
	// the middle of the address.
	//
	totalOffset =
	    sliceOffset + macroTileOffset + elementOffset + tileOffset;

	//
	// Get the pipe and bank.
	//

	// when the tileMode is PRT type, then adjust x and y
	// coordinates
	if (gpaIsPrt(arrayMode)) {
		x = x % macroTilePitch;
		y = y % macroTileHeight;
	}

	err = ComputePipeFromCoord(
	    x, y, slice, arrayMode, pipeSwizzle, pTileInfo, &pipe
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	err = ComputeBankFromCoord(
	    x, y, slice, arrayMode, bankSwizzle, tileSplitSlice, pTileInfo,
	    &bank
	);
	if (err != GPA_ERR_OK) {
		return err;
	}

	//
	// Split the offset to put some bits below the pipe+bank bits
	// and some above.
	//
	pipeInterleaveMask = (1 << numPipeInterleaveBits) - 1;
	bankInterleaveMask = (1 << numBankInterleaveBits) - 1;
	pipeInterleaveOffset = totalOffset & pipeInterleaveMask;
	bankInterleaveOffset =
	    (uint32_t)((totalOffset >> numPipeInterleaveBits) &
		       bankInterleaveMask);
	offset = totalOffset >> (numPipeInterleaveBits + numBankInterleaveBits);

	//
	// Assemble the address from its components.
	//
	addr = pipeInterleaveOffset;
	// This is to remove /analyze warnings
	uint32_t pipeBits = pipe << numPipeInterleaveBits;
	uint32_t bankInterleaveBits = bankInterleaveOffset
				      << (numPipeInterleaveBits + numPipeBits);
	uint32_t bankBits = bank
			    << (numPipeInterleaveBits + numPipeBits +
				numBankInterleaveBits);
	uint64_t offsetBits = offset
			      << (numPipeInterleaveBits + numPipeBits +
				  numBankInterleaveBits + numBankBits);

	addr |= pipeBits;
	addr |= bankInterleaveBits;
	addr |= bankBits;
	addr |= offsetBits;

	*pBytePosition = addr;
	return GPA_ERR_OK;
}

GpaError gpaComputeSurfaceCoord(
    uint64_t* outoffset, uint64_t* outbitoffset, const GpaSurfaceContext* ctx,
    uint32_t x, uint32_t y, uint32_t z, uint32_t fragindex
) {
	if (x > ctx->paddedwidth || y > ctx->paddedheight ||
	    z > ctx->paddeddepth || fragindex > ctx->numfragsperpixel) {
		return GPA_ERR_INVALID_ARGS;
	}

	const GnmArrayMode arraymode = gpaGetArrayMode(ctx->tilemode);
	const GnmMicroTileMode microTileType =
	    gpaGetMicroTileMode(ctx->tilemode);
	// ADDR_DEPTH_SAMPLE_ORDER = non-disp + depth-sample-order
	const bool isDepthSampleOrder =
	    microTileType == GNM_SURF_DEPTH_MICRO_TILING;

	if (ctx->mingpumode == GNM_GPU_NEO) {
		/// @note
		/// 128 bit/thick tiled surface doesn't support display
		/// tiling and mipmap chain must have the same tileType,
		/// so please fill tileType correctly
		if (!gpaIsLinear(arraymode)) {
			if (ctx->bitsperelement >= 128 ||
			    gpaGetMicroTileThickness(arraymode) > 1) {
				if (microTileType ==
				    GNM_SURF_DISPLAY_MICRO_TILING) {
					return GPA_ERR_INTERNAL_ERROR;
				}
			}
		}
	}

	GpaError err = GPA_ERR_OK;
	uint32_t bitPosition = 0;
	uint64_t addr = 0;

	switch (arraymode) {
	case GNM_ARRAY_LINEAR_GENERAL:
		addr = gpaComputeSurfaceAddrFromCoordLinear(
		    x, y, z, fragindex, ctx->bitsperelement, ctx->linearwidth,
		    ctx->linearheight, ctx->lineardepth, &bitPosition
		);
		break;
	case GNM_ARRAY_LINEAR_ALIGNED:
		addr = gpaComputeSurfaceAddrFromCoordLinear(
		    x, y, z, fragindex, ctx->bitsperelement, ctx->paddedwidth,
		    ctx->paddedheight, ctx->paddeddepth, &bitPosition
		);
		break;
	case GNM_ARRAY_1D_TILED_THIN1:	// fall through
	case GNM_ARRAY_1D_TILED_THICK:
		err = ComputeSurfaceAddrFromCoordMicroTiled(
		    x, y, z, fragindex, ctx->bitsperelement, ctx->paddedwidth,
		    ctx->paddedheight, ctx->paddeddepth, arraymode,
		    microTileType, isDepthSampleOrder, &addr, &bitPosition
		);
		break;
	case GNM_ARRAY_2D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_2D_TILED_THICK:	    // fall through
	case GNM_ARRAY_3D_TILED_THIN1:	    // fall through
	case GNM_ARRAY_3D_TILED_THICK:	    // fall through
	case GNM_ARRAY_2D_TILED_XTHICK:	    // fall through
	case GNM_ARRAY_3D_TILED_XTHICK:	    // fall through
	case GNM_ARRAY_PRT_TILED_THIN1:	    // fall through
	case GNM_ARRAY_PRT_2D_TILED_THIN1:  // fall through
	case GNM_ARRAY_PRT_3D_TILED_THIN1:  // fall through
	case GNM_ARRAY_PRT_TILED_THICK:	    // fall through
	case GNM_ARRAY_PRT_2D_TILED_THICK:  // fall through
	case GNM_ARRAY_PRT_3D_TILED_THICK:
		err = ComputeSurfaceAddrFromCoordMacroTiled(
		    x, y, z, fragindex, ctx->bitsperelement, ctx->paddedwidth,
		    ctx->paddedheight, ctx->paddeddepth, arraymode,
		    microTileType, isDepthSampleOrder, ctx->pipeswizzlemask,
		    ctx->bankswizzlemask, &ctx->tileinfo, &addr, &bitPosition
		);
		break;
	default:
		err = GPA_ERR_INTERNAL_ERROR;
		break;
	}

	if (outoffset) {
		*outoffset = addr;
	}
	if (outbitoffset) {
		*outbitoffset = bitPosition;
	}
	return err;
}

GpaError gpaComputeSurfaceSizeOffset(
    uint64_t* outsize, uint64_t* outoffset, const GpaTextureInfo* tex,
    uint32_t miplevel, uint32_t arrayslice
) {
	if (!tex) {
		return GPA_ERR_INVALID_ARGS;
	}

	const uint32_t numarrayslices = tex->numslices;
	const uint32_t basewidth = tex->width;
	const uint32_t baseheight = tex->height;
	const uint32_t basedepth = tex->depth;
	const uint32_t basepitch = tex->pitch;

	GpaTilingParams tp = {0};
	GpaError res = gpaTpInit(&tp, tex, 0, arrayslice);
	if (res != GPA_ERR_OK) {
		return res;
	}

	GpaSurfaceInfo si = {0};

	uint64_t finaloffset = 0;
	uint64_t finalsize = 0;

	for (uint32_t m = 0; m <= miplevel; m += 1) {
		finaloffset += numarrayslices * finalsize;

		tp.linearwidth = umax(basewidth >> m, 1);
		tp.linearheight = umax(baseheight >> m, 1);
		tp.lineardepth = basedepth;
		tp.basetiledpitch = basepitch;
		tp.miplevel = m;

		res = gpaComputeSurfaceInfo(&si, &tp);
		if (res != GPA_ERR_OK) {
			return res;
		}

		finalsize = si.surfacesize;
	}

	finaloffset += si.surfacesize * arrayslice;

	if (outsize) {
		*outsize = finalsize;
	}
	if (outoffset) {
		*outoffset = finaloffset;
	}
	return GPA_ERR_OK;
}
