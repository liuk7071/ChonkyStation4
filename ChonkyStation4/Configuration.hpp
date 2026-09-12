#pragma once

#include <Common.hpp>


namespace PS4::Configuration {

inline std::string system_dir_path      = "./system";
inline std::string system_ex_dir_path   = "./system_ex";
inline std::string sysmodules_path = "";

inline bool connect_to_network = false;
inline bool connect_to_psn = false;

inline bool is_vsh = false;

inline bool lle_ssl = false;

inline u32 gpu_device_id = 0;
inline float resolution_scale = 1.0f;
inline int shader_compiler_threads = 1;
inline bool shader_compiler_is_multithreaded = false;
inline bool precise_texure_offset = false;
inline bool precise_shader_movrel = false;

inline bool copy_command_buffers = false;
inline bool skip_async_compute_dispatches = false;
inline bool skip_waitregmem = false;
inline bool disable_gnmdetiler_texture_size = false;
inline bool disable_sgpr_init_hack = false;
inline bool clamp_gpu_buffers = false;
inline bool skip_bindless_buffers = false;

inline bool pipeline_dirty_state = false;
inline bool force_init_sce_compositor = false;

}   // End namespace PS4::Configuration