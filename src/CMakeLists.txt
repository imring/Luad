add_subdirectory(bclist)

add_executable(luad
    "plugins/bclist_types.cpp"
    "plugins/customfuncs.cpp"
    "plugins/dislua_types.cpp"
    "plugins/plugins.cpp"

    "disassembler.cpp"
    "file.cpp"
    "functions.cpp"
    "linehighlighter.cpp"
    "main.cpp"
    "mainwindow.cpp"
    "settings.cpp"
    "syntaxhighlighter.cpp"
    "utils.cpp"
    "variables.cpp"
    "xrefmenu.cpp"
)

include(FetchContent)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)

target_compile_features(luad PRIVATE cxx_std_20)
target_link_libraries(luad PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Widgets
    bclist
    qhexedit2
    ${LUA_LIBRARIES}
    sol2::sol2)
target_compile_definitions(luad PUBLIC
    $<$<CONFIG:DEBUG>:DEBUG_MODE>
    $<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>
)
target_include_directories(luad PRIVATE ${LUA_INCLUDE_DIR})