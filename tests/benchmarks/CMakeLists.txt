# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

# Phase 0 TEDS 基准评测 harness（验证/记录工具，非库交付物）。
# 门控 dtk6tablerecognizer 目标：仅当 onnxruntime 可用、库已构建时启用。

if(TARGET dtk${DTK_VERSION_MAJOR}tablerecognizer)
    set(TEDS_BIN teds_benchmark)
    add_executable(${TEDS_BIN} teds_benchmark.cpp)
    target_include_directories(${TEDS_BIN} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../src/tablerecognizer
        ${CMAKE_CURRENT_LIST_DIR}/../../include/dtktablerecognizer)
    target_link_libraries(${TEDS_BIN} PRIVATE
        dtk${DTK_VERSION_MAJOR}tablerecognizer
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets)
else()
    message(STATUS "teds_benchmark skipped: dtk${DTK_VERSION_MAJOR}tablerecognizer not built")
endif()
