// Luad - Disassembler for compiled Lua scripts.
// https://github.com/imring/Luad
// Copyright (C) 2021-2023 Vitaliy Vorobets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "utils.hpp"

std::size_t binary_search(const std::vector<bclist::div::line> &lines, std::size_t addr) {
    std::size_t low  = 0;
    std::size_t high = lines.size() - 1;

    while (low <= high) {
        const std::size_t mid = low + (high - low) / 2;
        const auto       &l   = lines[mid];
        if (l.from <= addr && l.to >= addr)
            return mid;
        if (l.from < addr)
            low++;
        else
            high--;
    }
    return bclist::max_line;
};

std::size_t utils::line_by_addr(const std::vector<bclist::div::line> &lines, std::size_t addr, bool last) {
    if (lines.empty()) {
        return bclist::max_line;
    }

    std::size_t res = binary_search(lines, addr);
    if (res != bclist::max_line) {
        if (last) {
            while (lines[res + 1].from <= addr && lines[res + 1].to >= addr) {
                res++;
            }
        } else {
            while (res != 0 && lines[res - 1].from <= addr && lines[res - 1].to >= addr) {
                res--;
            }
        }
    }
    return res;
}