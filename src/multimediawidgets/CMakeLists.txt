# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ${CMAKE_PROJECT_NAME}widgets)

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/${BIN_NAME}/*")
file(GLOB_RECURSE INCLUDE_FILES "./*.h"  "${INSTALL_INCLUDE_FILES}")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
)

include_directories(../../include)
include_directories(../../include/dtkmultimediawidgets)
include_directories(../../include/dtkmultimedia)
include_directories(common/utils)
include_directories(engine/interface)
include_directories(engine/multimedia)
include_directories(engine/multimedia/player)
include_directories(engine/plugin/ffmpeg)
include_directories(engine/plugin/gstreamer)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

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
    ${TARGET_LIBS}
    ${RSVG2_LIBRARIES}
)

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
