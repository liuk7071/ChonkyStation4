#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>


class App {
public:
    App() {}

    std::vector<Module> modules;

    void run();

private:
    MAKE_LOG_FUNCTION(log, loader_app);
};