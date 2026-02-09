#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <GCN/RegisterOffsets.hpp>
#include <GCN/FetchShader.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <SDL.h>


namespace PS4::GCN {

struct ColorTarget {
    bool enabled = false;
    int idx = 0;
    void* base = nullptr;
    u32 pitch_tile_max = 0;
    u32 slice_tile_max = 0;
    u32 width = 0;
    u32 height = 0;
    union {
        u32 raw = 0;
        BitField<0, 2, u32> endian;
        BitField<2, 5, u32> format;
        BitField<7, 1, u32> linear_general;
        BitField<8, 3, u32> number_type;
        BitField<11, 2, u32> comp_swap;
        BitField<13, 1, u32> fast_clear;
        BitField<14, 1, u32> compressed;
        BitField<15, 1, u32> blend_clamp;
        BitField<16, 1, u32> blend_bypass;
        BitField<17, 1, u32> simple_float;
        BitField<18, 1, u32> round_mode;
        BitField<19, 1, u32> cmask_is_linear;
        BitField<20, 3, u32> blend_opt_dont_read_dst;
        BitField<23, 3, u32> blend_opt_discard_pixeel;
        BitField<26, 1, u32> disable_fmask_compression;
    } info;

    ColorTarget operator=(const ColorTarget& other) {
        enabled = other.enabled;
        idx = other.idx;
        base = other.base;
        pitch_tile_max = other.pitch_tile_max;
        slice_tile_max = other.slice_tile_max;
        info.raw = other.info.raw;
        width = other.width;
        height = other.height;
        return *this;
    }

    bool operator==(const ColorTarget& other) {
        return enabled == other.enabled
            && idx == other.idx
            && base == other.base
            && pitch_tile_max == other.pitch_tile_max
            && slice_tile_max == other.slice_tile_max
            && info.raw == other.info.raw
            && width == other.width
            && height == other.height;
    }
};

struct DepthTarget {
    void* depth_base   = nullptr;
    void* stencil_base = nullptr;
    u32 width = 0;
    u32 height = 0;
    union {
        u32 raw;
        BitField<0,  2, u32> format;
        BitField<2,  2, u32> n_samples;
        BitField<13, 3, u32> tile_split;
        BitField<27, 1, u32> allow_expclear;
        BitField<28, 1, u32> read_size;
        BitField<29, 1, u32> tile_surface_enable;
        BitField<31, 1, u32> zrange_precision;
    } z_info;
    union {
        u32 raw;
        BitField<0,  1, u32> format;
        BitField<13, 3, u32> tile_split;
        BitField<27, 1, u32> allow_expclear;
        BitField<29, 1, u32> tile_stencil_disable;
    } stencil_info;

    DepthTarget operator=(const DepthTarget& other) {
        depth_base = other.depth_base;
        stencil_base = other.stencil_base;
        z_info.raw = other.z_info.raw;
        stencil_info.raw = other.stencil_info.raw;
        width = other.width;
        height = other.height;
        return *this;
    }

    bool operator==(const DepthTarget& other) {
        return depth_base == other.depth_base
            && stencil_base == other.stencil_base
            && z_info.raw == other.z_info.raw
            && stencil_info.raw == other.stencil_info.raw
            && width == other.width
            && height == other.height;
    }
};

// Used for a render target hack - more in CommandProcessor.cpp
union RenderTargetDimensions {
    u32 raw;

    struct {
        u16 width;
        u16 height;
    };
};

class Renderer {
public:
    Renderer() {
        std::memset(regs, 0, 0xd000);
    }

    SDL_Window* window;

    virtual void init() = 0;
    virtual void draw(const u64 cnt, const void* idx_buf_ptr = nullptr) = 0;
    virtual void flip(OS::Libs::SceVideoOut::SceVideoOutBuffer* buf) = 0;

    u32 regs[0xd000];
    RenderTargetDimensions color_rt_dim[8];
    RenderTargetDimensions depth_rt_dim;
    
    u8* getVSPtr() {
        return (u8*)(((u64)regs[Reg::mmSPI_SHADER_PGM_LO_VS] << 8) | ((u64)regs[Reg::mmSPI_SHADER_PGM_HI_VS] << 8 << 32));
    }

    u8* getPSPtr() {
        return (u8*)(((u64)regs[Reg::mmSPI_SHADER_PGM_LO_PS] << 8) | ((u64)regs[Reg::mmSPI_SHADER_PGM_HI_PS] << 8 << 32));
    }

    void getColorTargets(ColorTarget* rt) {
        // Render target info
        static const u32 base_reg_offsets[] = {
            Reg::mmCB_COLOR0_BASE,
            Reg::mmCB_COLOR1_BASE,
            Reg::mmCB_COLOR2_BASE,
            Reg::mmCB_COLOR3_BASE,
            Reg::mmCB_COLOR4_BASE,
            Reg::mmCB_COLOR5_BASE,
            Reg::mmCB_COLOR6_BASE,
            Reg::mmCB_COLOR7_BASE,
        };

        static const u32 pitch_reg_offsets[] = {
            Reg::mmCB_COLOR0_PITCH,
            Reg::mmCB_COLOR1_PITCH,
            Reg::mmCB_COLOR2_PITCH,
            Reg::mmCB_COLOR3_PITCH,
            Reg::mmCB_COLOR4_PITCH,
            Reg::mmCB_COLOR5_PITCH,
            Reg::mmCB_COLOR6_PITCH,
            Reg::mmCB_COLOR7_PITCH,
        };

        static const u32 slice_reg_offsets[] = {
            Reg::mmCB_COLOR0_SLICE,
            Reg::mmCB_COLOR1_SLICE,
            Reg::mmCB_COLOR2_SLICE,
            Reg::mmCB_COLOR3_SLICE,
            Reg::mmCB_COLOR4_SLICE,
            Reg::mmCB_COLOR5_SLICE,
            Reg::mmCB_COLOR6_SLICE,
            Reg::mmCB_COLOR7_SLICE,
        };

        static const u32 info_reg_offsets[] = {
            Reg::mmCB_COLOR0_INFO,
            Reg::mmCB_COLOR1_INFO,
            Reg::mmCB_COLOR2_INFO,
            Reg::mmCB_COLOR3_INFO,
            Reg::mmCB_COLOR4_INFO,
            Reg::mmCB_COLOR5_INFO,
            Reg::mmCB_COLOR6_INFO,
            Reg::mmCB_COLOR7_INFO,
        };


        // TODO: I'm not sure this register is the correct way to figure out which render targets are enabled.
        // It might be better to look at the PS shader exports directly.
        const u32 shader_col_mask = regs[Reg::mmCB_TARGET_MASK];
        for (int i = 0; i < 8; i++) {
            const bool is_enabled = (shader_col_mask >> (i * 4)) & 0xf;
            rt[i].enabled = is_enabled;
            rt[i].idx = i;
            rt[i].base = (void*)((u64)regs[base_reg_offsets[i]] << 8);
            rt[i].pitch_tile_max = regs[pitch_reg_offsets[i]];
            rt[i].slice_tile_max = regs[slice_reg_offsets[i]];
            rt[i].info.raw = regs[info_reg_offsets[i]];
            rt[i].width  = color_rt_dim[i].width;
            rt[i].height = color_rt_dim[i].height;
        }
    }

    void getDepthTarget(DepthTarget* depth, bool stencil_only) {
        if (regs[Reg::mmDB_Z_READ_BASE] != regs[Reg::mmDB_Z_WRITE_BASE]) {
            Helpers::panic("TODO: DB_Z_READ_BASE != DB_Z_WRITE_BASE");
        }

        if (regs[Reg::mmDB_STENCIL_READ_BASE] != regs[Reg::mmDB_STENCIL_WRITE_BASE]) {
            Helpers::panic("TODO: DB_STENCIL_READ_BASE != DB_STENCIL_WRITE_BASE");
        }

        if (regs[Reg::mmDB_Z_READ_BASE] != regs[Reg::mmDB_STENCIL_READ_BASE]) {
            //Helpers::panic("TODO: DB_Z_READ_BASE != DB_STENCIL_READ_BASE");
        }

        depth->depth_base       = (void*)((u64)regs[Reg::mmDB_Z_READ_BASE] << 8);
        depth->stencil_base     = (void*)((u64)regs[Reg::mmDB_STENCIL_READ_BASE] << 8);
        depth->z_info.raw       = regs[Reg::mmDB_Z_INFO];
        depth->stencil_info.raw = regs[Reg::mmDB_STENCIL_INFO];
        depth->width            = depth_rt_dim.width;
        depth->height           = depth_rt_dim.height;

        // Temporary hack
        if (stencil_only)
            depth->depth_base = depth->stencil_base;
    }
};

}   // End namespace GCN