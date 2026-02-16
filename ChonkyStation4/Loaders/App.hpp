#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <xbyak/xbyak.h>

#include <memory>
#include <tuple>
#include <deque>


class App {
public:
    App() {}

    std::string name;
    std::string title_id;
    std::deque<std::shared_ptr<Module>> modules;
    std::deque<std::string> unresolved_symbols;
    std::vector<std::unique_ptr<Xbyak::CodeGenerator>> unresolved_symbol_handlers;

    void run();
    std::tuple<u8*, size_t, size_t> getTLSImage(u32 modid);
    std::shared_ptr<Module> getHLEModule();

private:
    MAKE_LOG_FUNCTION(log, loader_app);
};