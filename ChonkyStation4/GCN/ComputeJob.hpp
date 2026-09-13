#pragma once

#include <Common.hpp>


namespace PS4::GCN {

struct ComputeJob {
    u32 dim_x = 0;
    u32 dim_y = 0;
    u32 dim_z = 0;
    u32 start_x = 0;
    u32 start_y = 0;
    u32 start_z = 0;
    u32 n_threads_x = 0;
    u32 n_threads_y = 0;
    u32 n_threads_z = 0;
    u32 lds_size_dwords = 0;
    u32 pgm_rsrc2 = 0;
    u32 n_user_sgprs = 0;
    bool tgid_x_en = false;
    bool tgid_y_en = false;
    bool tgid_z_en = false;
    void* addr = nullptr;
};

}   // End namespace PS4::GCN::Vulkan