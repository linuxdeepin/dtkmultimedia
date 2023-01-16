# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.10)
project(dtkocrpluginexample)
set(BIN_NAME load)

#for Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui)
find_package(Qt5 REQUIRED ${QtModule})

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)
add_definitions(${QT_DEFINITIONS})

include(CMakePackageConfigHelpers)

#for project files
include_directories(.)
aux_source_directory(. allSource)

#for ocr plugin manager
find_package(dtkocr)
include_directories(${dtkocr_INCLUDE_DIRS})

#link
add_library(${BIN_NAME} SHARED ${allSource})
target_link_libraries(${BIN_NAME} Qt5::Core Qt5::Gui)

#install
include(GNUInstallDirs)

#.so file
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/libdtkocr/plugins/${PROJECT_NAME})

#model file
set(SourcesDir ${CMAKE_INSTALL_PREFIX}/share)
set(CurrentProjectSourcesDir libdtkocr)
set(ModelSaveSubDir dtkocrpluginexamplemodel)
set(ModelInstallDir ${SourcesDir}/${CurrentProjectSourcesDir})
ADD_DEFINITIONS(-DOCR_MODEL_DIR="${CurrentProjectSourcesDir}/${ModelSaveSubDir}/")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${ModelSaveSubDir} DESTINATION ${ModelInstallDir})
