# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtk${DTK_VERSION_MAJOR}tablerecognizer)

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/dtktablerecognizer/*")
file(GLOB_RECURSE INCLUDE_FILES "./*.h")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)
include_directories(../../include/dtktablerecognizer)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Concurrent)
find_package(PkgConfig REQUIRED)

# 主依赖：onnxruntime（结构推理）、opencv_mobile（降级/预处理）为必需。
pkg_check_modules(OpenCVMobile IMPORTED_TARGET opencv_mobile)
pkg_check_modules(ORT IMPORTED_TARGET libonnxruntime)
if(NOT (ORT_FOUND AND OpenCVMobile_FOUND))
    message(STATUS
        "libdtk${DTK_VERSION_MAJOR}tablerecognizer: skipped (libonnxruntime/opencv_mobile dev not found)")
    return()
endif()

set(TARGET_LIBS
    Qt${QT_VERSION_MAJOR}::Gui
    PkgConfig::ORT
    PkgConfig::OpenCVMobile
)
add_library(${BIN_NAME} SHARED
    ${INSTALL_INCLUDE_FILES}
    ${INCLUDE_FILES}
    ${SRCS}
)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE
    VERSION="${CMAKE_PROJECT_VERSION}"
    DTK_BUILD_TABLERECOGNIZER_LIB
)
target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    ${PROJECT_INCLUDE}
)

# 复用 libdtk6ocr 公开 API（DOcr），不修改其源码。
# dtk6ocr 目标未通过 INTERFACE_INCLUDE_DIRECTORIES 导出头文件路径，需显式添加。
target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../include/dtkocr)
target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Concurrent
    dtk${DTK_VERSION_MAJOR}ocr
    ${TARGET_LIBS}
)

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC dtk${DTK_VERSION_MAJOR}ocr)
# config pkgconfig file
configure_file(${PROJECT_ROOT_DIR}/misc/dtk6tablerecognizer.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(${PROJECT_ROOT_DIR}/misc/dtk6tablerecognizerConfig.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(${PROJECT_ROOT_DIR}/misc/qt_lib_dtk6tablerecognizer.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")

# 模型随包分发到 /usr/share/libdtk6tablerecognizer/models/。
set(SourcesDir ${CMAKE_INSTALL_PREFIX}/share)
set(CurrentProjectSourcesDir lib${BIN_NAME})
set(ModelSaveSubDir models)

set(ModelInstallDir ${SourcesDir}/${CurrentProjectSourcesDir})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DTABLEREC_MODEL_DIR="${CMAKE_CURRENT_LIST_DIR}/${ModelSaveSubDir}/")
else()
    ADD_DEFINITIONS(-DTABLEREC_MODEL_DIR="${CurrentProjectSourcesDir}/${ModelSaveSubDir}/")
endif()

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${ModelSaveSubDir} DESTINATION ${ModelInstallDir})
