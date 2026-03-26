/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * symbolTable.hpp - Global string interning table for ultra-fast symbol matching
 */

#ifndef ROUCAARIZE_SYMBOL_TABLE_HPP
#define ROUCAARIZE_SYMBOL_TABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace roucaarize {

class SymbolTable {
public:
    static SymbolTable& get() {
        static SymbolTable instance;
        return instance;
    }

    uint32_t intern(const std::string& str) {
        auto it = strToId.find(str);
        if (it != strToId.end()) return it->second;
        uint32_t newId = static_cast<uint32_t>(strings.size());
        strings.push_back(str);
        strToId[str] = newId;
        return newId;
    }

    const std::string& getString(uint32_t id) const {
        static const std::string empty_str = "";
        if (id == UINT32_MAX || id >= strings.size()) return empty_str;
        return strings[id];
    }

private:
    std::unordered_map<std::string, uint32_t> strToId;
    std::vector<std::string> strings;
    
    SymbolTable() = default;
};

} // namespace roucaarize

#endif // ROUCAARIZE_SYMBOL_TABLE_HPP
