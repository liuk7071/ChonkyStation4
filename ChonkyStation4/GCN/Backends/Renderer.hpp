#pragma once

#include <Common.hpp>
#include <GCN/RegisterOffsets.hpp>
#include <GCN/FetchShader.hpp>
#include <SDL.h>


namespace PS4::GCN {

class Renderer {
public:
    Renderer() {
        std::memset(regs, 0, 0xd000);
    }

    SDL_Window* window;

    virtual void init() = 0;
    virtual void draw(const u64 cnt, const void* idx_buf_ptr = nullptr) = 0;
    virtual void flip() = 0;

    u32 regs[0xd000];
    
    u8* getVSPtr() {
        return (u8*)(((u64)regs[Reg::mmSPI_SHADER_PGM_LO_VS] << 8) | ((u64)regs[Reg::mmSPI_SHADER_PGM_HI_VS] << 8 << 32));
    }

    u8* getPSPtr() {
        return (u8*)(((u64)regs[Reg::mmSPI_SHADER_PGM_LO_PS] << 8) | ((u64)regs[Reg::mmSPI_SHADER_PGM_HI_PS] << 8 << 32));
    }
};

}   // End namespace GCN