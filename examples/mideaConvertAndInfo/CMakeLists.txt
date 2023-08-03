# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 2.project name
PROJECT(mideaConvertAndInfo CXX C)

find_package(Qt${QT_VERSION_MAJOR}Widgets)
find_package(Qt${QT_VERSION_MAJOR}Network)
find_package(Qt${QT_VERSION_MAJOR}MultimediaWidgets)

# 4.source directory
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# 5.set environment variable
SET(SRC_PATH ${DIR_SRCS})

if (ENABLE_Qt6)
    QT6_WRAP_UI (project_FORMS_HEADERS **.ui)
else()
    QT5_WRAP_UI (project_FORMS_HEADERS **.ui)
endif()

# 6.add executable file
file(GLOB_RECURSE INCLUDE_FILES "../../include/dtkmultimedia/*.h")
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_PATH} ${INCLUDE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include/dtkmultimedia
    ${PROJECT_SOURCE_DIR})

set(TARGET_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets)

# 7.add link library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${TARGET_LIBS} dtkmultimedia)

