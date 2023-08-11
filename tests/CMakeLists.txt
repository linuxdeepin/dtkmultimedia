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
    "../src/ocr/*.h"
    "../src/ocr/*.cpp"
    "${INCLUDE_FILES}"
    )
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

include_directories(
    "../include/dtkmultimedia"
    "../include/dtkmultimediawidgets"
    "../include/dtkocr"
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
include_directories(../src/ocr/ppocr)

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
    ${DFAKE_INTERFACE_DBUS_XML}
    )

find_package(Qt${QT_VERSION_MAJOR}Widgets)
find_package(Qt${QT_VERSION_MAJOR}DBus)
find_package(Qt${QT_VERSION_MAJOR}LinguistTools)
find_package(Qt${QT_VERSION_MAJOR}Network)
find_package(Qt${QT_VERSION_MAJOR}Concurrent)
find_package(Qt${QT_VERSION_MAJOR}Multimedia)
find_package(Qt${QT_VERSION_MAJOR}MultimediaWidgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(PkgConfig REQUIRED)

find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

if (NOT ENABLE_Qt6)
    find_package(Qt5X11Extras)
    pkg_check_modules(DBusextended REQUIRED IMPORTED_TARGET dbusextended-qt5)
endif()
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkwidget)
pkg_check_modules(Dtk REQUIRED IMPORTED_TARGET dtkcore)
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(InferenceEngine REQUIRED ncnn opencv_mobile)

set(TARGET_LIBS Xtst
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::Gst
    GL dl dtkmultimedia dtkocr)

list(APPEND PROJECT_INCLUDE /usr/include/glib-2.0 /usr/include/gstreamer-1.0)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    ${RSVG2_INCLUDE_DIRS}
    ${InferenceEngine_INCLUDE_DIRS}
    ${PROJECT_INCLUDE}
    )

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${RSVG2_LIBRARIES}
    ${InferenceEngine_LIBRARIES}
    ${TARGET_LIBS}
    -lpthread
    -lgcov
    -lgtest
    )
if (ENABLE_Qt6)
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::GuiPrivate
        )
else()
    target_link_libraries(${BIN_NAME} PRIVATE
        PkgConfig::DBusextended
        Qt5::X11Extras
        )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# OCR test setting and resource
set(ModelSaveSubDir dtkocrmodels)
ADD_DEFINITIONS(-DOCR_MODEL_DIR="${CMAKE_CURRENT_LIST_DIR}/../src/ocr/${ModelSaveSubDir}/")
add_subdirectory(ocrplugin)

add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
