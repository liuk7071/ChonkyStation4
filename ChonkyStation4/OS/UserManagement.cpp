#include "UserManagement.hpp"
#include <SDL.h>    // For SDL_GetPrefPath
#include <deque>


namespace PS4::OS::User {

static u32 next_user_id = 1;
std::deque<User> users;

User::User(u32 id, std::string username) : id(id), username(username) {
    // Ensure the home directory exists
    fs::create_directories(getHomeDir());
}

fs::path User::getHomeDir() {
    // TODO: Don't hardcode to PrefPath
    return fs::path(SDL_GetPrefPath("ChonkyStation", "ChonkyStation4")) / fs::path(std::format("users/{}_{}", id, username));
}

void init() {
    const auto user_dir = fs::path(SDL_GetPrefPath("ChonkyStation", "ChonkyStation4")) / "users";

    // Ensure the user directory exists
    fs::create_directories(user_dir);

    // Register existing users by iterating through the users directory
    u32 highest_id = 0;
    for (auto dir : fs::directory_iterator(user_dir)) {
        // User directories are named like "[id]_[username]"
        auto tokens = Helpers::split(dir.path().filename().generic_string(), "_");
        if (tokens.size() != 2) continue;

        const u32  id       = std::stoi(tokens[0]);
        const auto username = tokens[1];
        users.emplace_back(id, username);

        if (id > highest_id) highest_id = id;
    }
    next_user_id = highest_id + 1;
}

// Create a new user and return its id
u32 createNew(std::string username) {
    auto& user = users.emplace_back(next_user_id++, username);
    return user.getID();
}

// Returns whether or not the given user id exists
bool exists(u32 id) {
    for (auto& user : users) {
        if (user.getID() == id)
            return true;
    }
    return false;
}

// Returns nullptr if the user does not exist
User* getUser(u32 id) {
    for (auto& user : users) {
        if (user.getID() == id)
            return &user;
    }
    return nullptr;
}

// Returns whether or not the login was successful
bool login(u32 id) {
    current = getUser(id);
    return current != nullptr;
}

}   // End namespace PS4::OS::User