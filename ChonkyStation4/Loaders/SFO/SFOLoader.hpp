// Adapted from ChonkyStation3

#pragma once

#include <Common.hpp>
#include <Logger.hpp>

#include <unordered_map>


namespace PS4::Loader::SFO {

struct SFOData {
    std::unordered_map<std::string, std::u8string> strings;
    std::unordered_map<std::string, u32> ints;
};

struct SFOHeader {
    u32 magic; // Always " PSF"
    u32 version;
    u32 key_table_start;
    u32 data_table_start;
    u32 n_entries;
};

struct SFOIndexEntry {
    u16 key_offset;
    u16 data_fmt;
    u32 data_len;
    u32 data_max_len;
    u32 data_offset;
};

SFOData parse(fs::path path);
void save(SFOData& sfo_data, fs::path path);

}   // End namespace PS4::Loader::SFO
