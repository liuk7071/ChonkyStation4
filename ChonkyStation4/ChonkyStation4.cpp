#include <Common.hpp>
#include <PlayStation4.hpp>
#include <Configuration.hpp>
#include <OS/UserManagement.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <CLI11.hpp>
#include <SDL.h>


int main(int argc, char** argv) {
    // Try to reserve some address space as early as possible
#ifdef _WIN32
    void* ret = VirtualAlloc((void*)0x0'8000'0000, 2048_GB, MEM_RESERVE, PAGE_NOACCESS);
    if (!ret) {
        printf("Warning: failed to reserve address space\n");
    }
#endif
    
    std::string file;
    int uid = 1;
    std::string system_path = "./system";
    std::string system_ex_path = "./system_ex";
    std::string sysmodules_path = "";

    CLI::App cli_app = CLI::App();
    cli_app.add_option("game", file);   // For compatibility

    auto* run_cmd = cli_app.add_subcommand("run", "Run a game or executable");
    run_cmd->add_option("game", file, "Path to .elf, .self or game folder")->required();
    run_cmd->add_option("-u, --user", uid, "ID of the user to run the game with");
    run_cmd->add_option("--system-path", system_path, "Path of the /system directory");
    run_cmd->add_option("--system-ex-path", system_ex_path, "Path of the /system_ex directory");
    run_cmd->add_option("--sysmodules-path", sysmodules_path, "Path of the system modules");
    run_cmd->add_option("--connect-to-network", PS4::Configuration::connect_to_network, "Obtain an IP address and connect to the internet");
    run_cmd->add_option("--psn", PS4::Configuration::connect_to_psn, "Connect to the PSN Provider defined for the current user, if any");
    run_cmd->add_option("--lle-ssl", PS4::Configuration::lle_ssl, "LLE libSceSsl");
    run_cmd->add_option("--gpu", PS4::Configuration::gpu_device_id, "Choose GPU device ID");
    run_cmd->add_option("--resolution-scale", PS4::Configuration::resolution_scale, "Choose the resolution scale. This is experimental and won't work for most games");
    run_cmd->add_option("--shader-compiler-threads", PS4::Configuration::shader_compiler_threads, "Choose the number of concurrent shader compiler threads. 1 disables multithreading");
    run_cmd->add_option("--copy-command-buffers", PS4::Configuration::copy_command_buffers, "Copy GPU command buffers to separate buffers on submit");
    run_cmd->add_option("--skip-async-compute-dispatches", PS4::Configuration::skip_async_compute_dispatches, "Skip compute dispatches in async compute queues");
    run_cmd->add_option("--skip-waitregmem", PS4::Configuration::skip_waitregmem, "Skip the WaitRegMem packet");
    run_cmd->add_option("--disable-gnmdetiler-texture-size", PS4::Configuration::disable_gnmdetiler_texture_size, "Texture size calculation hack");
    run_cmd->add_option("--disable-sgpr-init-hack", PS4::Configuration::disable_sgpr_init_hack, "Disable SGPR init hack");
    run_cmd->add_option("--clamp-gpu-buffers", PS4::Configuration::clamp_gpu_buffers, "Clamp GPU buffer size to fit in mapped memory");
    run_cmd->add_option("--skip-bindless-buffers", PS4::Configuration::skip_bindless_buffers, "Skip bindless GPU buffers");
    run_cmd->add_option("--force-init-sce-compositor", PS4::Configuration::force_init_sce_compositor, "Initializes libSceComposite HLE even when not running VSH");

    auto* get_appdata_path_cmd = cli_app.add_subcommand("get_appdata_path", "Print the path to the emulator's app data folder");

    auto* user_cmd      = cli_app.add_subcommand("user", "Manage user accounts");
    auto* user_add_cmd  = user_cmd->add_subcommand("add");

    std::string user_add_username;
    user_add_cmd->add_option("name", user_add_username, "Name of the user to create")->required();

    CLI11_PARSE(cli_app, argc, argv);

    if (get_appdata_path_cmd->parsed()) {
        const fs::path path = SDL_GetPrefPath("ChonkyStation", "ChonkyStation4");
        fs::create_directories(path);
        std::puts(path.generic_string().c_str());
        return 0;
    }

    if (user_add_cmd->parsed()) {
        if (user_add_username.empty()) {
            Helpers::panic("No username specified\n");  // unreachable (name is required)
        }

        PS4::OS::User::init();
        PS4::OS::User::createNew(user_add_username);
        return 0;
    }

    printf("ChonkyStation4\n\n");
    
    PS4::OS::User::init();

    // Create the default user if it does not exist
    if (uid == 1 && !PS4::OS::User::exists(1))
        PS4::OS::User::createNew("ChonkyStation4");

    if (!PS4::OS::User::login(uid))
        Helpers::panic("Failed to login. User ID %d does not exist\n", uid);
    
    PS4::Configuration::system_dir_path = system_path;
    PS4::Configuration::system_ex_dir_path = system_ex_path;
    PS4::Configuration::sysmodules_path = sysmodules_path;

    PS4::Configuration::shader_compiler_is_multithreaded = PS4::Configuration::shader_compiler_threads > 1;

    fs::path file_path = file;
    PS4::loadAndRun(file);
    return 0;
}