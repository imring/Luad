set(QHEXEDIT_SRC "${CMAKE_CURRENT_LIST_DIR}/qhexedit2/src")

add_library(qhexedit2 STATIC
    ${QHEXEDIT_SRC}/qhexedit.cpp
    ${QHEXEDIT_SRC}/chunks.cpp
    ${QHEXEDIT_SRC}/commands.cpp
)
target_include_directories(qhexedit2 PUBLIC ${QHEXEDIT_SRC})
target_link_libraries(qhexedit2 PUBLIC
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Widgets)