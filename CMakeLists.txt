# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set (DTK_VERSION "1.0.0" CACHE STRING "define project version")

set(ENABLE_Qt6 CACHE BOOL OFF)
set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
set(BUILD_EXAMPLES ON CACHE BOOL "Build examples")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
endif()
if (ENABLE_Qt6)
    set(DTK_VERSION_MAJOR 6)
    set(QT_VERSION_MAJOR 6)
#    set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
    set(BUILD_DOCS OFF)
    set(BUILD_TESTING OFF)
    add_definitions(-DBUILD_Qt6)
else()
    set(QT_VERSION_MAJOR 5)
endif()

#set(LIB_NAME dtk${DTK_VERSION_MAJOR}multimedia)
set(LIB_NAME dtkmultimedia)

project(${LIB_NAME}
    VERSION ${DTK_VERSION}
    DESCRIPTION "DTK Multimedia module"
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkmultimedia
    LANGUAGES CXX C
)

# specify install dir
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
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

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(misc/${LIB_NAME}.pc.in ${LIB_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(misc/${LIB_NAME}Config.cmake.in ${LIB_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME})

# config qmake moudule file
set(DTK_MODULE ${LIB_NAME})
set(DTK_DEPS "core dbus")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(misc/qt_lib_${LIB_NAME}.pri.in qt_lib_${LIB_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${LIB_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
