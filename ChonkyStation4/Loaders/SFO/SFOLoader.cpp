// Adapted from ChonkyStation3

#include "SFOLoader.hpp"


namespace PS4::Loader::SFO {

MAKE_LOG_FUNCTION(log, loader_sfo);

#include "SFOLoader.hpp"


SFOData parse(fs::path path) {
    SFOData sfo_data;
    log("Parsing SFO %s\n", path.generic_string().c_str());

    const auto sfo = Helpers::readBinary(path);
    SFOHeader* header = (SFOHeader*)sfo.data();

    if (*(u32*)&header->magic != *(u32*)"\0PSF")
        Helpers::panic("%s is not a valid SFO file\n", path.generic_string().c_str());

    for (int i = 0; i < header->n_entries; i++) {
        SFOIndexEntry* entry = (SFOIndexEntry*)&sfo[sizeof(SFOHeader) + (sizeof(SFOIndexEntry) * i)];
        const auto key = Helpers::readString(&sfo[header->key_table_start + entry->key_offset]);
        switch (entry->data_fmt) {
        // String / Array
        case 0x004:
        case 0x204: {
            const auto data = std::u8string(reinterpret_cast<const char8_t*>(&sfo[header->data_table_start + entry->data_offset]));
            log("{ %s : %s }\n", key.c_str(), data.c_str());
            sfo_data.strings[key] = data;
            break;
        }
        // Integer
        case 0x404: {
            const u32 data = *(u32*)&sfo[header->data_table_start + entry->data_offset];
            log("{ %s : 0x%08x }\n", key.c_str(), data);
            sfo_data.ints[key] = data;
            break;
        }
        default:
            Helpers::panic("SFO: unimplemented data format type 0x%04x\n", entry->data_fmt);
        }
    }

    return sfo_data;
}

void save(SFOData& sfo_data, fs::path path) {
    u32 sfo_size;
    // Allocate a bigger than enough buffer (most SFOs don't go past like 3kb)
    u8* buf = new u8[512_KB];
    std::memset(buf, 0, 512_KB);
    log("Saving SFO %s\n", path.generic_string().c_str());
    
    SFOHeader* header = (SFOHeader*)buf;
    *(u32*)&header->magic = *(u32*)"\0PSF";
    header->version = 0x101;
    
    header->n_entries = sfo_data.strings.size() + sfo_data.ints.size();
    header->key_table_start = sizeof(SFOHeader) + header->n_entries * sizeof(SFOIndexEntry);
    
    log("Number of entries: %d\n", (u32)header->n_entries);
    
    // Compute size of key table
    u32 key_table_size = 0;
    for (auto& [key, val] : sfo_data.strings)
        key_table_size += key.length() + 1; // +1 to include null terminator
    for (auto& [key, val] : sfo_data.ints)
        key_table_size += key.length() + 1;
    header->data_table_start = header->key_table_start + key_table_size;
    
    u32 cur_key_offs = 0;
    u32 cur_data_offs = 0;
    
    int i = 0;
    
    // Save strings
    for (auto& [key, val] : sfo_data.strings) {
        SFOIndexEntry* entry = (SFOIndexEntry*)&buf[sizeof(SFOHeader) + (sizeof(SFOIndexEntry) * i)];
        
        // Write key
        entry->key_offset = cur_key_offs;
        std::strcpy((char*)&buf[header->key_table_start + entry->key_offset], key.c_str());
        cur_key_offs += key.length() + 1;
        // Write data
        entry->data_offset = cur_data_offs;
        std::strcpy((char*)&buf[header->data_table_start + entry->data_offset], (char*)val.c_str());
        cur_data_offs += val.length() + 1;
        // Set format
        entry->data_fmt = 0x204; // String
        entry->data_len = val.length();
        entry->data_max_len = val.length() + 1;
        
        i++;
    }
    
    // Save integers
    for (auto& [key, val] : sfo_data.ints) {
        SFOIndexEntry* entry = (SFOIndexEntry*)&buf[sizeof(SFOHeader) + (sizeof(SFOIndexEntry) * i)];
        
        // Write key
        entry->key_offset = cur_key_offs;
        std::strcpy((char*)&buf[header->key_table_start + entry->key_offset], key.c_str());
        cur_key_offs += key.length() + 1;
        // Write data
        entry->data_offset = cur_data_offs;
        *(u32*)&buf[header->data_table_start + entry->data_offset] = val;
        cur_data_offs += sizeof(u32);
        // Set format
        entry->data_fmt = 0x404; // Integer
        entry->data_len = 4;
        entry->data_max_len = entry->data_len;
        
        i++;
    }
    
    // Write to file
    std::ofstream file = std::ofstream(path, std::ios::binary);
    file.write((char*)buf, header->data_table_start + cur_data_offs);
}


}   // End namespace PS4::Loader::SFO