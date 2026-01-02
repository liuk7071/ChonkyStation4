#pragma once

#include <Common.hpp>


class App;
class Module;

namespace PS4::Loader::Linker {

    ::App loadAndLink(const fs::path& path);
    void doRelocations(::App& app);
    void loadAndLinkLib(::App& app, const fs::path& path, bool is_partial_lle_module = false, Module* hle_module = nullptr);

} // End namespace Loader::Linker