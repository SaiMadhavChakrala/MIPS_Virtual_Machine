#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <cstdint>

class SymbolEntry {
    public:
    std::string name;
    uint8_t type;
    uint8_t binding;
    uint8_t defined;
    uint32_t address;
};

#endif