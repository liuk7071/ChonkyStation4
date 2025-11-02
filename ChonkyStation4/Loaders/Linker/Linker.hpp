#pragma once

#include <Common.hpp>
#include <Loaders/App.hpp>


namespace Loader::Linker {

    App loadAndLink(const fs::path& path);

} // End namespace Loader::Linker