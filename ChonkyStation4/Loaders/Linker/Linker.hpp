#pragma once

#include <Common.hpp>


class App;

namespace Loader::Linker {

    App loadAndLink(const fs::path& path);

} // End namespace Loader::Linker