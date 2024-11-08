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

#ifndef LUAD_PLUGINS_HPP
#define LUAD_PLUGINS_HPP

#include <QObject>

#include <filesystem>
#include <sol/sol.hpp>

#include "../file.hpp"

class LuaPluginManager;

struct LuaPlugin {
    LuaPlugin(LuaPluginManager *m) : manager{m} {};
    ~LuaPlugin();

    LuaPluginManager     *manager = nullptr;
    std::filesystem::path path;
    sol::state            state;

    template <typename T>
    requires(std::is_base_of_v<sol::proxy_base<T>, T>) bool valid_result(const T &result);

    bool run();
    void message(std::string_view text);
};

class LuaPluginManager : public QObject {
    Q_OBJECT

public:
    LuaPluginManager(QObject *parent = nullptr) : QObject{parent} {}

    enum class MessageType {
        Info,
        Script,
        Error,
    };
    void message(MessageType type, std::string_view text);
    void error(LuaPlugin *plugin, std::string_view text);

    bool loadPlugin(std::filesystem::path path);
    void loadPlugins();

    static LuaPluginManager *instance();

signals:
    void onMessage(std::string_view text);

public slots:
    void openFile(std::weak_ptr<File> file);

private:
    std::weak_ptr<File>                   file;
    std::list<std::shared_ptr<LuaPlugin>> plugins;
};

template <typename T>
requires(std::is_base_of_v<sol::proxy_base<T>, T>) bool LuaPlugin::valid_result(const T &result) {
    if (!result.valid()) {
        sol::error err = result;
        manager->error(this, err.what());
        return false;
    }
    return true;
}

#endif // LUAD_PLUGINS_HPP