#pragma once

#include <Common.hpp>


class App;
class Module;

namespace PS4::Loader::Linker {
    
    ::App loadAndLink(const fs::path& path);
    void doRelocations(::App& app);
    std::shared_ptr<Module> loadAndLinkLib(::App& app, const fs::path& path, bool is_partial_lle_module = false, std::shared_ptr<Module> hle_module = nullptr);

} // End namespace Loader::Linker