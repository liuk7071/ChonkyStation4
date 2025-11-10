#pragma once

#include <Common.hpp>


#define DSEL_0 0
#define DSEL_1 1
#define DSEL_R 4
#define DSEL_G 5
#define DSEL_B 6
#define DSEL_A 7

// BUF_DATA_FORMAT
#define BUF_DATA_FORMAT_INVALID 0x00
#define BUF_DATA_FORMAT_8 0x01
#define BUF_DATA_FORMAT_16 0x02
#define BUF_DATA_FORMAT_8_8 0x03
#define BUF_DATA_FORMAT_32 0x04
#define BUF_DATA_FORMAT_16_16 0x05
#define BUF_DATA_FORMAT_10_11_11 0x06
#define BUF_DATA_FORMAT_11_11_10 0x07
#define BUF_DATA_FORMAT_10_10_10_2 0x08
#define BUF_DATA_FORMAT_2_10_10_10 0x09
#define BUF_DATA_FORMAT_8_8_8_8 0x0a
#define BUF_DATA_FORMAT_32_32 0x0b
#define BUF_DATA_FORMAT_16_16_16_16 0x0c
#define BUF_DATA_FORMAT_32_32_32 0x0d
#define BUF_DATA_FORMAT_32_32_32_32 0x0e
#define BUF_DATA_FORMAT_RESERVED 0x0f

// BUF_NUM_FORMAT
#define BUF_NUM_FORMAT_UNORM 0x00
#define BUF_NUM_FORMAT_SNORM 0x01
#define BUF_NUM_FORMAT_USCALED 0x02
#define BUF_NUM_FORMAT_SSCALED 0x03
#define BUF_NUM_FORMAT_UINT 0x04
#define BUF_NUM_FORMAT_SINT 0x05
#define BUF_NUM_FORMAT_SNORM_NZ 0x06
#define BUF_NUM_FORMAT_FLOAT 0x07

struct VSharp {
    u64 base : 40;
    u64 unused : 4;
    u64 mtype_L1s : 2;
    u64 mtype_L2 : 2;
    u64 stride : 14; //bytes: 0..16383
    u64 cache_swizzle : 1;
    u64 swizzle_en : 1;  //swizzle AOS according to stride, index_stride, and element_size, else linear (stride * index + offset)
    u32 num_records:32; //n units of 'stride'
    u32 dst_sel_x : 3;  //Destination channel select:
    u32 dst_sel_y : 3;  //0=0, 1=1, 4=R, 5=G, 6=B, 7=A
    u32 dst_sel_z : 3;
    u32 dst_sel_w : 3;
    u32 nfmt : 3;  //numeric data type (float, int, ...)
    u32 dfmt : 4;  //data format
    u32 element_size : 2;  //2, 4, 8, or 16  bytes. Used for swizzled buffer addressing
    u32 index_stride : 2;  //8, 16, 32, or 64. Used for swizzled buffer addressing
    u32 addtid_en : 1;  //addtid_en add thread id to the index for addr calc
    u32 reserved1 : 1;
    u32 hash_en : 1;
    u32 reserved2 : 1;
    u32 mtype : 3;
    u32 _type : 2; //value == 0 for buf. Overlaps upper 2 bits of 4-bit TYPE field in 128-bit T# resource

    int getVMemoryType() {
        return (this->mtype_L1s << 5) |
               (this->mtype_L2) |
               (this->mtype << 2);

    };

    void setVMemoryType(int m) {
        this->mtype_L1s = (m >> 5);
        this->mtype_L2 = m;
        this->mtype = (m >> 2);
    };

    void setVMemoryPtrs(void* ptr, u32 offset, u32 stride, u32 count) {
        this->base        = ((uptr)ptr) + offset;
        this->stride      = stride;
        this->num_records = count;
    };
};
                           
#define VMemoryTypePV 0x60 ///< Private         
#define VMemoryTypeGC 0x6D ///< GPU Coherent    
#define VMemoryTypeSC 0x6E ///< System Coherent 
#define VMemoryTypeUC 0x6F ///< Uncached        
#define VMemoryTypeRO 0x10 ///< Read Only       