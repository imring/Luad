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

#include "plugins.hpp"

#include <fmt/chrono.h>

#include "../mainwindow.hpp"
#include "customfuncs.hpp"

namespace fs = std::filesystem;

const std::unordered_map<LuaPluginManager::MessageType, std::string> message_type_strings = {
    { LuaPluginManager::MessageType::Info, "(info)" },
    { LuaPluginManager::MessageType::Script, "(script)" },
    { LuaPluginManager::MessageType::Error, "(error)" }
};

LuaPlugin::~LuaPlugin() {
    manager->message(LuaPluginManager::MessageType::Info, std::format("Unloading the plugin {}...", path.filename().string()));
}

bool LuaPlugin::run() {
    // load
    sol::load_result load_result = state.load_file(path.string());
    bool result = valid_result(load_result);
    if (result) {
        // run
        sol::protected_function script_func = load_result.get<sol::protected_function>();
        result = valid_result(script_func());
    }
    return result;
}

void LuaPlugin::message(std::string_view text) {
    manager->message(LuaPluginManager::MessageType::Script, std::format("{}: {}", path.filename().string(), text));
}

void LuaPluginManager::openFile(std::weak_ptr<File> f) {
    file = f;

    for (auto &plugin: plugins) {
        const sol::protected_function &func = plugin->state["on_open_file"];
        if (func.valid()) {
            plugin->valid_result(func(file.lock()));
        }
    }
}

void LuaPluginManager::message(MessageType type, std::string_view text) {
    const std::string string_type = message_type_strings.at(type);
    const auto now = std::chrono::system_clock::now();
    emit onMessage(fmt::format("[{:%d.%m.%Y %H:%M:%S}] {:s} {:s}", now, string_type, text));
}

void LuaPluginManager::error(LuaPlugin *plugin, std::string_view reason) {
    message(LuaPluginManager::MessageType::Error, std::format("Error running plugin {}: {}", plugin->path.filename().string(), reason));

    plugins.remove_if([plugin](const std::shared_ptr<LuaPlugin> &p) {
        return p.get() == plugin;
    });
}

bool LuaPluginManager::loadPlugin(std::filesystem::path path) {
    auto result  = std::make_shared<LuaPlugin>(this);
    result->path = path;

    message(MessageType::Info, std::format("Loading plugin {}...", path.filename().string()));

    const std::string spath = path.string();
    LuaCustom::initialize(*result);
    bool running = result->run();
    if (running) {
        plugins.emplace_back(std::move(result));
    }
    return running;
}

void LuaPluginManager::loadPlugins() {
    constexpr std::string_view directory = "plugins";
    constexpr std::string_view extension = ".lua";

    for (const auto &entry: fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            loadPlugin(entry.path());
        }
    }
}

LuaPluginManager *LuaPluginManager::instance() {
    static LuaPluginManager *singleton = nullptr;
    if (!singleton) {
        // must use operator new for Qt destructor
        singleton = new LuaPluginManager;
    }
    return singleton;
}