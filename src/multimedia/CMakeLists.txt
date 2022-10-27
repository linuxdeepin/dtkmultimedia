# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/${BIN_NAME}/*")
file(GLOB_RECURSE INCLUDE_FILES "./*.h")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)
include_directories(../../include/${BIN_NAME})


add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
)

include_directories(ffmpeg)
include_directories(gstreamer)
include_directories(common/algorithm)
include_directories(common/interface)
include_directories(common/utils)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

find_package(Qt5Gui)
find_package(Qt5X11Extras)
find_package(Qt5Multimedia)


set(TARGET_LIBS Qt5::X11Extras Qt5::Multimedia Qt5::Gui)

list(APPEND PROJECT_INCLUDE /usr/include/glib-2.0 /usr/include/gstreamer-1.0)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    ${RSVG2_INCLUDE_DIRS}
    ${PROJECT_INCLUDE}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${TARGET_LIBS}
    ${RSVG2_LIBRARIES}
)

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
