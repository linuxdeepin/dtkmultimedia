# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.10)
project(ocrdebugger)

#for Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Widgets)
find_package(Qt5 REQUIRED ${QtModule})

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
)
add_definitions(${QT_DEFINITIONS})

include(CMakePackageConfigHelpers)

#for project files
include_directories(.)
aux_source_directory(. allSource)

#for dtkocr
file(GLOB_RECURSE INCLUDE_FILES "../../include/dtkocr/*.h")

add_executable(${PROJECT_NAME} ${allSource})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include/dtkocr
    ${PROJECT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} dtkocr Qt5::Core Qt5::Gui Qt5::Widgets)
