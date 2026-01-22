#pragma once

#include <Common.hpp>
#include <GCN/GCN.hpp>


namespace PS4::GCN {

void initCommandProcessor();
void processCommands(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size);

}   // End namespace PS4::GCN