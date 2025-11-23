#pragma once

#include <Common.hpp>
#include <Loaders/App.hpp>


namespace PS4::Loader::App {

struct AppInfo {
    std::string title;
    std::string version;

    fs::path content_path;
    fs::path param_sfo_path;
    fs::path executable_path;
    fs::path icon0_path;
    fs::path pic1_path;
};

bool prepareApp(const fs::path& app_content_path, AppInfo& info);
bool getApp(const AppInfo& info, ::App& app);

}   // End namespace PS4::Loader::App