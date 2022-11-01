# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkmultimedia)

#add_definitions(-DUSE_MOCK_INTERFACE)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

include(./dbus/fakeInterface.cmake)

file(GLOB_RECURSE INCLUDE_FILES "../include/*.h")
file(GLOB_RECURSE SRCS
    "../src/multimediawidgets/*.h"
    "../src/multimediawidgets/*.cpp"
    "${INCLUDE_FILES}"
)
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

include_directories(
    "../include/dtkmultimedia"
    "../include/dtkmultimediawidgets"
    "../src/multimediawidgets/dbus"
    "3rdparty/cpp-stub"
    "3rdparty/stub-ext"
)

include_directories(../src/multimediawidgets/common/utils)
include_directories(../src/multimediawidgets/engine/interface)
include_directories(../src/multimediawidgets/engine/multimedia)
include_directories(../src/multimediawidgets/engine/multimedia/player)
include_directories(../src/multimediawidgets/engine/plugin/ffmpeg)
include_directories(../src/multimediawidgets/engine/plugin/gstreamer)

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
    ${DFAKE_INTERFACE_DBUS_XML}
)

find_package(Qt5Widgets)
find_package(Qt5DBus)
find_package(Qt5X11Extras)
find_package(Qt5LinguistTools)
find_package(Qt5Network)
find_package(Qt5Concurrent)
find_package(Qt5Sql)
find_package(Qt5Svg)
find_package(Qt5Multimedia)
find_package(Qt5MultimediaWidgets)
find_package(Qt5Xml)
find_package(PkgConfig REQUIRED)

pkg_search_module(RSVG2 REQUIRED librsvg-2.0)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(Xcb REQUIRED IMPORTED_TARGET xcb xcb-shape)
pkg_check_modules(DBusextended REQUIRED IMPORTED_TARGET dbusextended-qt5)
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gobject-2.0)
# IMPORTED_TARGET failed to work for some of libs under flatpak env
pkg_check_modules(Other REQUIRED  gsettings-qt)

set(TARGET_LIBS X11 Xtst PkgConfig::Xcb Qt5::Widgets Qt5::X11Extras Qt5::Network Qt5::DBus Qt5::Sql Qt5::Svg Qt5::Multimedia
    Qt5::MultimediaWidgets PkgConfig::Dtk PkgConfig::DBusextended PkgConfig::Gst GL va va-x11
    Qt5::Concurrent Qt5::Xml dtkmultimedia)

list(APPEND PROJECT_INCLUDE /usr/include/glib-2.0 /usr/include/gstreamer-1.0)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    ${RSVG2_INCLUDE_DIRS}
    ${PROJECT_INCLUDE}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    ${RSVG2_LIBRARIES}
    ${TARGET_LIBS}
    -lpthread
    -lgcov
    -lgtest
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

enable_testing()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
