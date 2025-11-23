#pragma once

#include <Common.hpp>


class App;

namespace PS4::Loader::Linker {

    ::App loadAndLink(const fs::path& path);
    void doRelocations(::App& app);
    void loadAndLinkLib(::App& app, const fs::path& path);

} // End namespace Loader::Linker