# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ${LIB_NAME}widgets)

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/dtkmultimediawidgets/*")
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

find_package(Qt${QT_VERSION_MAJOR}Widgets)
find_package(Qt${QT_VERSION_MAJOR}DBus)
find_package(Qt${QT_VERSION_MAJOR}LinguistTools)
find_package(Qt${QT_VERSION_MAJOR}Network)
find_package(Qt${QT_VERSION_MAJOR}Concurrent)
find_package(Qt${QT_VERSION_MAJOR}Multimedia)
find_package(Qt${QT_VERSION_MAJOR}MultimediaWidgets)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS GuiPrivate REQUIRED)
endif()
if (ENABLE_Qt6)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets)
else()
    find_package(Qt5X11Extras)
endif()
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat)

set(TARGET_LIBS Xtst
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::Gst GL
    dtk${DTK_VERSION_MAJOR}multimedia)

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
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${TARGET_LIBS}
    ${RSVG2_LIBRARIES}
    PkgConfig::FFMPEG
    )

if (ENABLE_Qt6)
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        )
else()
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt5::X11Extras
        )
endif()

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
