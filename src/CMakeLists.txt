add_subdirectory(bclist)

add_executable(luad
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

target_compile_features(luad PRIVATE cxx_std_20)
target_link_libraries(luad PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Widgets
    bclist
    qhexedit2)
target_compile_definitions(luad PUBLIC
    $<$<CONFIG:DEBUG>:DEBUG_MODE>
    $<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>
)