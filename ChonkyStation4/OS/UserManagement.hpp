#pragma once

#include <Common.hpp>


namespace PS4::OS::User {

class User {
public:
    User(u32 id, std::string username);

    u32         getID()       { return id; }
    std::string getUsername() { return username; }
    fs::path getHomeDir();

private:
    u32 id;
    std::string username;
};

inline User* current = nullptr;

void init();
u32 createNew(std::string username);
bool exists(u32 id);
User* getUser(u32 id);
bool login(u32 id);

}   // End namespace PS4::OS::User