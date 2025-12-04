#pragma once

#include <Common.hpp>


// AMD Sea Islands Instruction Set Architecture, table 8.13
enum class DataFormat : u32 {
    FormatInvalid = 0,
    Format8 = 1,
    Format16 = 2,
    Format8_8 = 3,
    Format32 = 4,
    Format16_16 = 5,
    Format10_11_11 = 6,
    Format11_11_10 = 7,
    Format10_10_10_2 = 8,
    Format2_10_10_10 = 9,
    Format8_8_8_8 = 10,
    Format32_32 = 11,
    Format16_16_16_16 = 12,
    Format32_32_32 = 13,
    Format32_32_32_32 = 14,
    Format5_6_5 = 16,
    Format1_5_5_5 = 17,
    Format5_5_5_1 = 18,
    Format4_4_4_4 = 19,
    Format8_24 = 20,
    Format24_8 = 21,
    FormatX24_8_32 = 22,
    FormatGB_GR = 32,
    FormatBG_RG = 33,
    Format5_9_9_9 = 34,
    FormatBc1 = 35,
    FormatBc2 = 36,
    FormatBc3 = 37,
    FormatBc4 = 38,
    FormatBc5 = 39,
    FormatBc6 = 40,
    FormatBc7 = 41,
    FormatFmask8_1 = 47,
    FormatFmask8_2 = 48,
    FormatFmask8_4 = 49,
    FormatFmask16_1 = 50,
    FormatFmask16_2 = 51,
    FormatFmask32_2 = 52,
    FormatFmask32_4 = 53,
    FormatFmask32_8 = 54,
    FormatFmask64_4 = 55,
    FormatFmask64_8 = 56,
    Format4_4 = 57,
    Format6_5_5 = 58,
    Format1 = 59,
    Format1_Reversed = 60,
    Format32_As_8 = 61,
    Format32_As_8_8 = 62,
    Format32_As_32_32_32_32 = 63,
};

enum class NumberFormat : u32 {
    Unorm = 0,
    Snorm = 1,
    Uscaled = 2,
    Sscaled = 3,
    Uint = 4,
    Sint = 5,
    SnormNz = 6,
    Float = 7,
    Srgb = 9,
    Ubnorm = 10,
    UbnormNz = 11,
    Ubint = 12,
    Ubscaled = 13,
};

struct TSharp {
    u64 base_address : 38;
    u64 mtype_l2 : 2;
    u64 min_lod : 12;
    u64 data_format : 6;
    u64 num_format : 4;
    u64 mtype : 2;
    u64 width : 14;
    u64 height : 14;
    u64 perf_modulation : 3;
    u64 interlaced : 1;
    u64 dst_sel_x : 3;
    u64 dst_sel_y : 3;
    u64 dst_sel_z : 3;
    u64 dst_sel_w : 3;
    u64 base_level : 4;
    u64 last_level : 4;
    u64 tiling_index : 5;
    u64 pow2pad : 1;
    u64 mtype2 : 1;
    u64 atc : 1;
    u64 type : 4; // overlaps with V# type
    
    u64 depth : 13;
    u64 pitch : 14;
    u64 : 5;
    u64 base_array : 13;
    u64 last_array : 13;
    u64 : 6;
    u64 min_lod_warn : 12;
    u64 counter_bank_id : 8;
    u64 lod_hw_cnt_en : 1;
    
    /// Neo-mode only
    u64 compression_en : 1;
    u64 alpha_is_on_msb : 1;
    u64 color_transform : 1;
    u64 alt_tile_mode : 1;
    u64 : 39;
};