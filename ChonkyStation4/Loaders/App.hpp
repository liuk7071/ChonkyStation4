#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <xbyak/xbyak.h>

#include <memory>
#include <tuple>
#include <deque>


struct Params {
    int argc;
    u32 padding;
    const char* argv[33];
    void* entry;
};

class App {
public:
    App() {}

    std::string name;
    std::string title_id;
    std::deque<std::shared_ptr<Module>> modules;
    std::deque<std::string> unresolved_symbols;
    std::vector<std::unique_ptr<Xbyak::CodeGenerator>> unresolved_symbol_handlers;

    Params params;

    void run();
    std::tuple<u8*, size_t, size_t> getTLSImage(u32 modid);
    std::shared_ptr<Module> getHLEModule();
    std::shared_ptr<Module> findModule(s32 modid);
    std::shared_ptr<Module> findModuleByName(const std::string& name);
    std::shared_ptr<Module> findModuleByAddress(void* addr);
    void forEachModule(std::function<bool(std::shared_ptr<Module>)> func);

private:
    MAKE_LOG_FUNCTION(log, loader_app);
};