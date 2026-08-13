#pragma once

#include <Common.hpp>


namespace PS4::Configuration {

inline std::string system_dir_path      = "./system";
inline std::string system_ex_dir_path   = "./system_ex";
inline std::string sysmodules_path = "";

inline bool is_vsh = false;

inline bool lle_ssl = false;

inline u32 gpu_device_id = 0;
inline float resolution_scale = 1.0f;

inline bool copy_command_buffers = false;
inline bool skip_async_compute_dispatches = false;
inline bool skip_waitregmem = false;
inline bool disable_gnmdetiler_texture_size = false;
inline bool disable_sgpr_init_hack = false;
inline bool clamp_gpu_buffers = false;
inline bool skip_bindless_buffers = false;

}   // End namespace PS4::Configuration