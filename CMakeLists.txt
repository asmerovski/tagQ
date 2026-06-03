cmake_minimum_required(VERSION 3.16)
project(tagQ VERSION 0.0.1 LANGUAGES CXX)
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

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF6 COMPONENTS 
    CoreAddons
    WidgetsAddons
    KIO
    ConfigWidgets
    Codecs
    REQUIRED
)

find_package(TagLib REQUIRED)

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
    KF6::CoreAddons
    KF6::WidgetsAddons
    KF6::KIO
    KF6::ConfigWidgets
    KF6::Codecs
    TagLib::TagLib
)

target_include_directories(tagQ PRIVATE ${TAGLIB_INCLUDE_DIRS})

install(TARGETS tagQ DESTINATION bin)
install(FILES src/tagQ.desktop DESTINATION share/applications)
install(FILES src/tagQ.svg DESTINATION share/icons/hicolor/scalable/apps)
