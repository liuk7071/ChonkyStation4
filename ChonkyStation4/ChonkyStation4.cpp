#include <Common.hpp>
#include <PlayStation4.hpp>
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

    CLI::App cli_app = CLI::App();
    cli_app.add_option("game", file);   // For compatibility

    auto* run_cmd = cli_app.add_subcommand("run", "Run a game or executable");
    run_cmd->add_option("game", file, "Path to .elf, .self or game folder")->required();
    run_cmd->add_option("-u, --user", uid, "ID of the user to run the game with");

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
    
    fs::path file_path = file;
    PS4::loadAndRun(file);
    return 0;
}