#pragma once

#include <Common.hpp>
#include <GCN/GCN.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>


namespace PS4::GCN {

void initCommandProcessor();
void processCommands(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size, OS::Libs::SceGnmDriver::ComputeQueue* compute_queue, bool is_indirect = false);

}   // End namespace PS4::GCN