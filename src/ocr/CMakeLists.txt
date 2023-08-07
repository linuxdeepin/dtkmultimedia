# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtkocr)

file(GLOB_RECURSE INSTALL_INCLUDE_FILES "../../include/${BIN_NAME}/*")
file(GLOB_RECURSE INCLUDE_FILES "./*.h" "${INSTALL_INCLUDE_FILES}")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)
include_directories(../../include/${BIN_NAME})

find_package(Qt${QT_VERSION_MAJOR}Gui)
find_package(PkgConfig REQUIRED)

set(TARGET_LIBS Qt${QT_VERSION_MAJOR}::Gui)

add_library(${BIN_NAME} SHARED
    ${INSTALL_INCLUDE_FILES}
    ${INCLUDE_FILES}
    ${SRCS}
)

include_directories(interface)
include_directories(ppocr)

pkg_check_modules(InferenceEngine REQUIRED ncnn opencv_mobile)
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    ${PROJECT_INCLUDE}
    ${InferenceEngine_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${TARGET_LIBS}
    ${InferenceEngine_LIBRARIES}
)

install(FILES ${INSTALL_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)
# config pkgconfig file
configure_file(${PROJECT_ROOT_DIR}/misc/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(${PROJECT_ROOT_DIR}/misc/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(${PROJECT_ROOT_DIR}/misc/qt_lib_${BIN_NAME}.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")

# config the deep learning models of the default plugin
set(SourcesDir ${CMAKE_INSTALL_PREFIX}/share)
set(CurrentProjectSourcesDir lib${BIN_NAME})
set(ModelSaveSubDir dtkocrmodels)
set(ModelInstallDir ${SourcesDir}/${CurrentProjectSourcesDir})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DOCR_MODEL_DIR="${CMAKE_CURRENT_LIST_DIR}/${ModelSaveSubDir}/")
else()
    ADD_DEFINITIONS(-DOCR_MODEL_DIR="${CurrentProjectSourcesDir}/${ModelSaveSubDir}/")
endif()

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${ModelSaveSubDir} DESTINATION ${ModelInstallDir})
