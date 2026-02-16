#pragma once

#include <Common.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::HLE {

std::shared_ptr<Module> buildHLEModule();

}   // End namespace PS4::OS::HLE