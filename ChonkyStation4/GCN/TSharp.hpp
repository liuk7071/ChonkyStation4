#pragma once

#include <Common.hpp>


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