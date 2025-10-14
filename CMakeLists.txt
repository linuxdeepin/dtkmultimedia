# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# If do't define version number, specify the version number
set (DTK_VERSION "1.0.0" CACHE STRING "define project version")

project(dtkmultimedia
    VERSION ${DTK_VERSION}
    DESCRIPTION "DTK Multimedia module"
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkmultimedia
    LANGUAGES CXX C
)

set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
set(BUILD_EXAMPLES ON CACHE BOOL "Build examples")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
endif()

# specify install dir
include(GNUInstallDirs)

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
# 检查并设置环境变量
if(NOT DEFINED ENV{QT_SELECT})
    set(ENV{QT_SELECT} "qt${QT_VERSION_MAJOR}")
endif()
message("   >>> Build with Qt version: $ENV{QT_SELECT}")

if (QT_VERSION_MAJOR MATCHES 6)
    set(DTK_VERSION_MAJOR 6)
    add_definitions(-DBUILD_Qt6)
    set(ENABLE_Qt6 ON CACHE BOOL "Build with Qt6")
else()
    set(DTK_VERSION_MAJOR "")
    set(ENABLE_Qt6 OFF CACHE BOOL "Build without Qt6")
endif()
message("   >>> Build with DTK: ${DTK_VERSION_MAJOR}")

set(LIB_NAME dtk${DTK_VERSION_MAJOR}multimedia)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fno-omit-frame-pointer")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui DBus)

add_subdirectory(src)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(examples/player)
    add_subdirectory(examples/audioRecorder)
    add_subdirectory(examples/camera)
    add_subdirectory(examples/screenRecorder)
    add_subdirectory(examples/screenShot)
    add_subdirectory(examples/ocrCmd)
    add_subdirectory(examples/ocrDebugger)
    add_subdirectory(examples/mideaConvertAndInfo)
endif ()

if (BUILD_DOCS STREQUAL ON)
    find_package(Doxygen)
    if (BUILD_DOCS AND DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif ()
endif()

# test
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()
