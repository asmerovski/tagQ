cmake_minimum_required(VERSION 3.16)

project(tagQ VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Concurrent
    REQUIRED
)

find_package(KDE6 COMPONENTS 
    CoreAddons
    WidgetsAddons
    KIO
    ConfigGui
    Codecs
    REQUIRED
)

find_package(Taglib REQUIRED)

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

target_link_libraries(tagQ
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    KDE6::CoreAddons
    KDE6::WidgetsAddons
    KDE6::KIO
    KDE6::ConfigGui
    KDE6::Codecs
    tag
)

target_include_directories(tagQ PRIVATE ${TAGLIB_INCLUDE_DIRS})

install(TARGETS tagQ DESTINATION bin)
install(FILES src/tagQ.desktop DESTINATION share/applications)
install(FILES src/tagQ.svg DESTINATION share/icons/hicolor/scalable/apps)
