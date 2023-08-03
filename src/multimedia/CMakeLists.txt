# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ${CMAKE_PROJECT_NAME})
project(${BIN_NAME} CXX C)

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/${BIN_NAME}/*")
#file(GLOB_RECURSE INCLUDE_FILES "./*.h" "${INSTALL_INCLUDE_FILES}")
#file(GLOB_RECURSE SRCS
#    "./*.h"
#    "./*.c"
#    "./*.cpp"
#)
file(GLOB_RECURSE AUDIORECORDER_SRCS
    "./audiorecorder/*.h"
    "./audiorecorder/*.c"
    "./audiorecorder/*.cpp"
)
file(GLOB_RECURSE CAMERA_SRCS
    "./camera/*.h"
    "./camera/*.c"
    "./camera/*.cpp"
)
file(GLOB_RECURSE COMMON_SRCS
    "./common/*.h"
    "./common/*.c"
    "./common/*.cpp"
)
file(GLOB_RECURSE FFMPEG_SRCS
    "./ffmpeg/*.h"
    "./ffmpeg/*.c"
    "./ffmpeg/*.cpp"
)
file(GLOB_RECURSE GSTREAMER_SRCS
    "./gstreamer/*.h"
    "./gstreamer/*.c"
    "./gstreamer/*.cpp"
)
file(GLOB_RECURSE INFOANDCONV_SRCS
    "./multimediainfoandconv/*.h"
    "./multimediainfoandconv/*.c"
    "./multimediainfoandconv/*.cpp"
)
file(GLOB_RECURSE SCREENRECORDER_SRCS
    "./screenrecorder/*.h"
    "./screenrecorder/*.c"
    "./screenrecorder/*.cpp"
)


include_directories(../../include/${BIN_NAME})
include_directories(ffmpeg)
include_directories(gstreamer)
include_directories(common/algorithm)
include_directories(common/interface)
include_directories(common/utils)
include_directories(camera/libcam/libcam_v4l2core)
include_directories(camera/libcam/libcam_render)
include_directories(camera/libcam/libcam_encoder)
include_directories(camera/libcam/libcam_audio)
include_directories(camera/libcam/libcam)
include_directories(camera/libcam)

add_library(${BIN_NAME} SHARED
#    ${INCLUDE_FILES}
#    ${SRCS}
    ${INSTALL_INCLUDE_FILES}
    ${AUDIORECORDER_SRCS}
    ${CAMERA_SRCS}
    ${COMMON_SRCS}
    ${FFMPEG_SRCS}
    ${GSTREAMER_SRCS}
    ${INFOANDCONV_SRCS}
    ${SCREENRECORDER_SRCS}
)


set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

find_package(Qt${QT_VERSION_MAJOR}Gui)
find_package(Qt${QT_VERSION_MAJOR}Multimedia)


set(TARGET_LIBS Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Gui)

list(APPEND PROJECT_INCLUDE /usr/include/glib-2.0 /usr/include/gstreamer-1.0 /usr/include/libusb-1.0)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    ${RSVG2_INCLUDE_DIRS}
    ${PROJECT_INCLUDE}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    ${TARGET_LIBS}
    ${RSVG2_LIBRARIES}
    )

if (ENABLE_Qt6)
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        )
endif()

if("${QT_VERSION_MAJOR}" STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS X11Extras)
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt5::X11Extras
        )
endif()

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
