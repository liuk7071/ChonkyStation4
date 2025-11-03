#pragma once

#include <Common.hpp>


class Symbol {
public:
    Symbol() {}

    std::string nid;
    std::string name = "Unknown";
    std::string lib;
    std::string module;
    std::string ver;
    void* ptr;
};