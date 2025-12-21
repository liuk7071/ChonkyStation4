#ifndef _GPUADDR_PRIVATE_H_
#define _GPUADDR_PRIVATE_H_

#include "gpuaddr.h"

#include "utility.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Common constants
////////////////////////////////////////////////////////////////////////////////////////////////////
static const uint32_t MicroTileWidth =
    8;	///< Micro tile width, for 1D and 2D tiling
static const uint32_t MicroTileHeight =
    8;	///< Micro tile height, for 1D and 2D tiling
static const uint32_t ThickTileThickness =
    4;	///< Micro tile thickness, for THICK modes
static const uint32_t XThickTileThickness =
    8;	///< Extra thick tiling thickness
static const uint32_t PowerSaveTileBytes =
    64;	 ///< Nuber of bytes per tile for power save 64
static const uint32_t CmaskCacheBits =
    1024;				  ///< Number of bits for CMASK cache
static const uint32_t CmaskElemBits = 4;  ///< Number of bits for CMASK element
static const uint32_t HtileCacheBits =
    16384;  ///< Number of bits for HTILE cache 512*32

static const uint32_t MicroTilePixels = MicroTileWidth * MicroTileHeight;

static const uint32_t Block64K = 0x10000;
static const uint32_t PrtTileSize = Block64K;

static const uint32_t PIPE_INTERLEAVE_BYTES = 256;
static const uint32_t BANK_INTERLEAVE = 1;

#define BLOCK_SIZE 4
#define MICROTILE_SIZE 8
#define TILE_SIZE 8
#define DRAM_ROW_SIZE 1024

static inline uint32_t QLog2(uint32_t x) {
	uint32_t y = 0;

	switch (x) {
	case 1:
		y = 0;
		break;
	case 2:
		y = 1;
		break;
	case 4:
		y = 2;
		break;
	case 8:
		y = 3;
		break;
	case 16:
		y = 4;
		break;
	}

	return y;
}

static inline bool IsPow2(const uint32_t x) {
	return (x > 0) && ((x & (x - 1)) == 0);
}
static inline uint32_t NextPow2(uint32_t x) {
	x = x - 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}
static inline uint32_t PowTwoAlign32(uint32_t x, uint32_t align) {
	return (x + (align - 1)) & (~(align - 1));
}

static inline uint32_t BitsToBytes32(uint32_t x) {
	return (x + (8 - 1)) / 8;
}
static inline uint64_t BitsToBytes64(uint64_t x) {
	return (x + (8 - 1)) / 8;
}
static inline uint32_t BytesToBits32(uint32_t x) {
	return x * 8;
}
static inline uint64_t BytesToBits64(uint64_t x) {
	return x * 8;
}

GnmArrayMode gpaGetArrayMode(GnmTileMode tilemode);
GnmMicroTileMode gpaGetMicroTileMode(GnmTileMode tilemode);
GnmPipeConfig gpaGetPipeConfig(GnmTileMode tilemode);
GnmPipeConfig gpaGetAltPipeConfig(GnmTileMode tilemode);
GnmSampleSplit gpaGetSampleSplit(GnmTileMode tilemode);
GnmTileSplit gpaGetTileSplit(GnmTileMode tilemode);

GpaError gpaCalcSurfaceMacrotileMode(
    GnmMacroTileMode* outmtm, GnmTileMode tilemode, uint32_t bitsperelem,
    uint32_t numfragsperpixel
);

GpaError gpaAdjustTileMode(
    GnmTileMode* outtilemode, GnmTileMode oldtilemode, GnmArrayMode newarraymode
);

uint32_t gpaGetMicroTileThickness(GnmArrayMode arraymode);
bool gpaIsLinear(GnmArrayMode arraymode);
bool gpaIsMicroTiled(GnmArrayMode arraymode);
bool gpaIsMacroTiled(GnmArrayMode arraymode);
bool gpaIsPrt(GnmArrayMode arraymode);

GnmBankWidth gpaGetBankWidth(GnmMacroTileMode mtm);
//
// BASE mode macrotilemode stuff
//
GnmBankHeight gpaGetBankHeight(GnmMacroTileMode mtm);
GnmNumBanks gpaGetNumBanks(GnmMacroTileMode mtm);
GnmMacroTileAspect gpaGetMacrotileAspect(GnmMacroTileMode mtm);
//
// NEO mode macrotilemode stuff
//
GnmBankHeight gpaGetAltBankHeight(GnmMacroTileMode mtm);
GnmNumBanks gpaGetAltNumBanks(GnmMacroTileMode mtm);
GnmMacroTileAspect gpaGetAltMacrotileAspect(GnmMacroTileMode mtm);

uint32_t gpaGetPipeCount(GnmPipeConfig pipecfg);

static inline uint32_t getblockpitch(const GnmDataFormat fmt) {
	const uint32_t bytesperelem = gnmDfGetTotalBytesPerElement(fmt);
	const uint32_t texelsperelemwide = gnmDfGetTexelsPerElementWide(fmt);
	return BLOCK_SIZE * bytesperelem / texelsperelemwide;
}
static inline uint32_t gettilepitch(const GnmDataFormat fmt) {
	const uint32_t bytesperelem = gnmDfGetTotalBytesPerElement(fmt);
	const uint32_t texelsperelemwide = gnmDfGetTexelsPerElementWide(fmt);
	return TILE_SIZE * bytesperelem / texelsperelemwide;
}

static inline uint32_t getelemsperblockwide(const GnmDataFormat fmt) {
	const uint32_t elemwidth = gnmDfGetTexelsPerElementWide(fmt);
	return BLOCK_SIZE / elemwidth;
}
static inline uint32_t getelemsperblocktall(const GnmDataFormat fmt) {
	const uint32_t elemheight = gnmDfGetTexelsPerElementTall(fmt);
	return BLOCK_SIZE / elemheight;
}

static inline uint32_t GetTileSplitBytes(
    GnmTileSplit split, uint32_t bpp, uint32_t thickness
) {
	uint32_t tileBytes1x = BitsToBytes32(bpp * MicroTilePixels * thickness);

	// Non-depth entries store a split factor
	uint32_t sampleSplit = 64 << split;
	return umax(256u, sampleSplit * tileBytes1x);
}

uint64_t gpaComputeSurfaceAddrFromCoordLinear(
    uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp,
    uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t* pBitPosition
);

#endif	// _GPUADDR_PRIVATE_H_
