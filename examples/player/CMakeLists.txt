# Generated from player.pro.
# 1.cmake verson，指定cmake版本
cmake_minimum_required(VERSION 3.13)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 2.project name
PROJECT(player)


find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5MultimediaWidgets)

# 4.source directory
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# 5.set environment variable
SET(SRC_PATH ${DIR_SRCS})

# 6.add executable file
file(GLOB_RECURSE INCLUDE_FILES "../../include/*.h")
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_PATH} ${INCLUDE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../../include/dtkmultimedia
    ${PROJECT_SOURCE_DIR}/../../include/dtkmultimediawidgets
    ${PROJECT_SOURCE_DIR}/../../include/dtkocr)

set(TARGET_LIBS Qt5::Widgets Qt5::Network Qt5::MultimediaWidgets)

# 7.add link library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${TARGET_LIBS} dtkmultimedia dtkmultimediawidgets dtkocr)

