#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <xbyak/xbyak.h>

#include <memory>
#include <deque>


class App {
public:
    App() {}

    std::vector<Module> modules;
    std::deque<std::string> unresolved_symbols;
    std::vector<std::unique_ptr<Xbyak::CodeGenerator>> unresolved_symbol_handlers;

    void run();
    std::pair<u8*, size_t> getTLSImage();

private:
    MAKE_LOG_FUNCTION(log, loader_app);
};