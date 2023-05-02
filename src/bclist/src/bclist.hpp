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

#ifndef BCLIST_H
#define BCLIST_H

#include <map>
#include <memory>

#include <fmt/format.h>

#include "dislua/dislua.hpp"

class bclist {
public:
    inline static constexpr size_t max_line = static_cast<size_t>(-1);

    struct options {
        // Maximum line length (default: 50). Number 0 remove line break.
        size_t max_length;

        explicit options(size_t ml = 50) : max_length(ml) {}
    };

    explicit bclist(dislua::dump_info *i, const options &op = options{}) : info{i}, option{op} {}
    virtual ~bclist() {
        delete info;
    };

    struct div {
        struct line {
            std::string text;
            std::string key;
            size_t      from;
            size_t      to;

            explicit line(std::string_view text = {}, size_t from = 0, size_t to = 0, std::string_view key = {}) : text{text}, from{from}, to{to}, key{key} {}
        };

        std::string       key;
        size_t            tab = 0;
        std::string       header, footer;
        std::vector<line> lines;
        std::vector<div>  additional;

        template <typename... Args>
        void new_line(size_t from = bclist::max_line, size_t size = 0, std::string_view str = {}, Args&&... args);
        template <typename... Args>
        void new_line(std::string_view key, size_t from = bclist::max_line, size_t size = 0, std::string_view str = {}, Args&&... args);
        void empty_line(size_t p = bclist::max_line);
        void add_div(const div &d);

        [[nodiscard]] std::string string(bool from = false) const;
        [[nodiscard]] div         only_lines() const;
        [[nodiscard]] bool        empty() const;

        [[nodiscard]] size_t start() const;
        [[nodiscard]] size_t end() const;
    };

    [[nodiscard]] bool is_newline(size_t len) const {
        return option.max_length != 0 && len > option.max_length;
    }
    [[nodiscard]] std::string full() const {
        return divs.string();
    }
    virtual void update() {}

    // FIXME
    template <typename... Args>
    void new_line(div &d, size_t size, std::string_view str, Args&&... args) {
        d.new_line<Args...>(offset, size, str, std::forward<Args>(args)...);
        offset += size;
    }
    template <typename... Args>
    void new_line(div &d, std::string_view key, size_t size, std::string_view str, Args&&... args) {
        d.new_line<Args...>(key, offset, size, str, std::forward<Args>(args)...);
        offset += size;
    }

    void add_ref(std::size_t key, std::size_t value);
    void add_ref(std::size_t key, const std::vector<std::size_t> &values);

    std::map<size_t, std::vector<size_t>> refs;
    div                                   divs;
    dislua::dump_info                    *info;
    options                               option;

    static std::unique_ptr<bclist> get_list(const dislua::dump_info &info);

protected:
    size_t offset = 0;
};

template <typename... Args>
void bclist::div::new_line(size_t from, size_t size, std::string_view str, Args&&... args) {
    const size_t to = size == 0 ? from : from + size - 1;
    if constexpr (sizeof...(args) == 0)
        lines.emplace_back(str, from, to);
    else
        lines.emplace_back(fmt::format(fmt::runtime(str), std::forward<Args>(args)...), from, to);
}

template <typename... Args>
void bclist::div::new_line(std::string_view key, size_t from, size_t size, std::string_view str, Args&&... args) {
    const size_t to = size == 0 ? from : from + size - 1;
    if constexpr (sizeof...(args) == 0)
        lines.emplace_back(str, from, to, key);
    else
        lines.emplace_back(fmt::format(fmt::runtime(str), std::forward<Args>(args)...), from, to, key);
}

#endif // BCLIST_H