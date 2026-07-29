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

# Phase 0 表格结构检测单测由独立的 ut-tablerecognizer 目标构建（见文件末尾），
# 从 ut-dtkmultimedia glob 中剔除，避免在未编译 tablerecognizer 源码时产生未定义符号。
list(FILTER TEST_FILES EXCLUDE REGEX "ut_ortinferenceengine|ut_tablestructuredetector")

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

# 找到基础模块
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets DBus LinguistTools Network Concurrent Multimedia MultimediaWidgets OpenGL)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

# 根据Qt版本处理OpenGLWidgets
if(${QT_VERSION_MAJOR} EQUAL 6)
    # Qt6需要单独查找OpenGLWidgets模块
    find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
endif()

find_package(PkgConfig REQUIRED)
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

if (NOT ENABLE_Qt6)
    find_package(Qt5X11Extras)
endif()
pkg_check_modules(Gst REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(InferenceEngine REQUIRED ncnn opencv_mobile)

set(TARGET_LIBS Xtst
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::OpenGL
    PkgConfig::Gst
    GL dl
    dtk${DTK_VERSION_MAJOR}multimedia
    dtk${DTK_VERSION_MAJOR}ocr)

# 仅在Qt6时链接OpenGLWidgets
if(${QT_VERSION_MAJOR} EQUAL 6)
    list(APPEND TARGET_LIBS Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

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


# ===== Phase 0: 表格结构检测单元测试（独立可验证子集）=====
# 门控 dtk6tablerecognizer 目标（即 onnxruntime 可用）；仅接 ut_ortinferenceengine +
# ut_tablestructuredetector。其余 5 个 tablerecognizer 测试属 Phase 1，本阶段不引入。
if(TARGET dtk${DTK_VERSION_MAJOR}tablerecognizer)
    set(UT_TR_NAME ut-tablerecognizer)
    add_executable(${UT_TR_NAME}
        ut_ortinferenceengine.cpp
        ut_tablestructuredetector.cpp)
    target_include_directories(${UT_TR_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../src/tablerecognizer
        ${CMAKE_CURRENT_LIST_DIR}/../include/dtktablerecognizer)
    find_package(GTest)
    if(GTest_FOUND)
        set(UT_TR_GTEST GTest::gtest GTest::gtest_main)
    else()
        set(UT_TR_GTEST gtest gtest_main)
    endif()
    target_link_libraries(${UT_TR_NAME} PRIVATE
        dtk${DTK_VERSION_MAJOR}tablerecognizer
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        ${UT_TR_GTEST}
        pthread)
    add_test(NAME ${UT_TR_NAME} COMMAND ${UT_TR_NAME})
else()
    message(STATUS "ut-tablerecognizer skipped: dtk${DTK_VERSION_MAJOR}tablerecognizer not built (onnxruntime unavailable)")
endif()

# Phase 0 TEDS 基准 harness（验证/记录工具）。
add_subdirectory(benchmarks)
