# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.10)
project(tablerecognizercmd)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QtModule Core Gui Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED ${QtModule})

include_directories(.)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include/dtktablerecognizer
    ${PROJECT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    dtk${DTK_VERSION_MAJOR}tablerecognizer
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Concurrent)
