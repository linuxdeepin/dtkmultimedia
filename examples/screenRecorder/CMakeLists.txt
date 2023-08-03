# Generated from audioRecorder.pro.
# 1.cmake verson，指定cmake版本
cmake_minimum_required(VERSION 3.13)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 2.project name
PROJECT(screenRecorder)

find_package(Qt${QT_VERSION_MAJOR}Widgets)
find_package(Qt${QT_VERSION_MAJOR}Network)
find_package(Qt${QT_VERSION_MAJOR}MultimediaWidgets)

# 4.source directory
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# 5.set environment variable
SET(SRC_PATH ${DIR_SRCS})

# 6.add executable file
file(GLOB_RECURSE INCLUDE_FILES "../../include/dtkmultimedia/*.h")
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_PATH} ${INCLUDE_FILES})


target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include/dtkmultimedia
    ${PROJECT_SOURCE_DIR})

set(TARGET_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets)

# 7.add link library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${TARGET_LIBS} dtkmultimedia)
