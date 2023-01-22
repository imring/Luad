add_subdirectory(bclist)

add_executable(luad
    "editor/editor.cpp"
    "editor/functions.cpp"
    "editor/highlighter.cpp"
    "editor/variables.cpp"

    "main.cpp"
    "mainwindow.cpp"
    "settings.cpp"
)

target_compile_features(luad PRIVATE cxx_std_20)
target_link_libraries(luad PRIVATE
    Qt${QT_VERSION}::Widgets
    bclist)
target_compile_definitions(luad PUBLIC
    $<$<CONFIG:DEBUG>:DEBUG_MODE>
    $<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>
)