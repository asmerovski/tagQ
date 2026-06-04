cmake_minimum_required(VERSION 3.16)
project(tagQ VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)

# ── TagLib ────────────────────────────────────────────────────────────────────
# Prefer a pre-built static copy (set -DTAGLIB_STATIC_PREFIX=... at configure
# time). Falls back to pkg-config (system dynamic library).
set(TAGLIB_STATIC_PREFIX "" CACHE PATH "Path to a static TagLib install prefix")

if(EXISTS "${TAGLIB_STATIC_PREFIX}/lib/libtag.a")
    message(STATUS "tagQ: using static TagLib from ${TAGLIB_STATIC_PREFIX}")
    add_library(TagLib::tag STATIC IMPORTED)
    set_target_properties(TagLib::tag PROPERTIES
        IMPORTED_LOCATION             "${TAGLIB_STATIC_PREFIX}/lib/libtag.a"
        INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_STATIC_PREFIX}/include"
    )
    set(TAGLIB_LIBRARIES TagLib::tag z)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(TAGLIB REQUIRED taglib)
    message(STATUS "tagQ: using system TagLib ${TAGLIB_VERSION}")
endif()
# ─────────────────────────────────────────────────────────────────────────────

set(tagQ_SOURCES
    src/main.cpp
    src/application.h
    src/application.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/filebrowser.h
    src/filebrowser.cpp
    src/filelist.h
    src/filelist.cpp
    src/metadataeditor.h
    src/metadataeditor.cpp
    src/audiofile.h
    src/audiofile.cpp
    src/tagsmanager.h
    src/tagsmanager.cpp
    src/resources.qrc
)

add_executable(tagQ ${tagQ_SOURCES})

target_include_directories(tagQ PRIVATE
    src
    ${TAGLIB_INCLUDE_DIRS}
)

target_link_libraries(tagQ PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    ${TAGLIB_LIBRARIES}
)

if(TAGLIB_CFLAGS_OTHER)
    target_compile_options(tagQ PRIVATE ${TAGLIB_CFLAGS_OTHER})
endif()

install(TARGETS tagQ DESTINATION bin)
install(FILES src/tagQ.desktop DESTINATION share/applications)
