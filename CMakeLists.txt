cmake_minimum_required(VERSION 3.16)
project(UEFIViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

# Специальные флаги для Qt6 и GCC 15
add_compile_options(-fPIC)
# Включить PIC только на Linux
if (UNIX AND NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Явное указание пути к Qt6 (для Manjaro/Arch)
# set(Qt6_DIR "/usr/lib/cmake/Qt6")
# Поиск Qt6 (без хардкода путей)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Поиск Qt6 компонентов
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# QHexEdit как обычная библиотека (не STATIC)
add_library(qhexedit
    lib/qhexedit2/src/qhexedit.cpp
    lib/qhexedit2/src/chunks.cpp
    lib/qhexedit2/src/commands.cpp
    lib/qhexedit2/src/color_manager.cpp
)

target_include_directories(qhexedit PUBLIC
    lib/qhexedit2/src
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
)

# Основное приложение
add_executable(UEFIViewer
    main.cpp
    mainwindow.cpp
    mainwindow.h
    signrequest.h
    signrequest.cpp
    signrequest.ui
)

target_link_libraries(UEFIViewer PRIVATE
    Qt6::Core
    Qt6::Widgets
    qhexedit
)

# Специальные флаги линковки для Qt6
if (UNIX)
    target_link_options(UEFIViewer PRIVATE
        -Wl,--copy-dt-needed-entries
        -no-pie
    )
endif()
