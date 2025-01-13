# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#${CMAKE_INSTALL_DATAROOTDIR}/qt5/doc

if (ENABLE_Qt6)
    find_package(Qt6 COMPONENTS ToolsTools CONFIG REQUIRED)
    if (TARGET Qt6::qhelpgenerator)
        get_target_property(QHelpGenerator_EXECUTABLE Qt6::qhelpgenerator LOCATION)
        set (DOXYGEN_QHG_LOCATION "${QHelpGenerator_EXECUTABLE}")
    else ()
        message ("error: can't found qhelpgenerator from Qt6 tools.")
    endif ()
else ()
    set (DOXYGEN_QHG_LOCATION "qhelpgenerator")
endif ()

set (QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt${QT_VERSION_MAJOR}/doc CACHE STRING "QCH install location")
set (DOXYGEN_GENERATE_HTML "YES" CACHE STRING "Doxygen HTML output")
set (DOXYGEN_GENERATE_XML "NO" CACHE STRING "Doxygen XML output")
set (DOXYGEN_GENERATE_QHP "YES" CACHE STRING "Doxygen QHP output")
set (DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set (DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set (DOXYGEN_EXTRACT_STATIC YES)
set (DOXYGEN_OUTPUT_LANGUAGE "Chinese")
set (DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set (DOXYGEN_TAGFILES "qtcore.tags=qthelp://doc.qt.io/qt-${QT_VERSION_MAJOR}/" CACHE STRING "Doxygen tag files")
set (DOXYGEN_MACRO_EXPANSION "YES")
set (DOXYGEN_EXPAND_ONLY_PREDEF "YES")

#Multimedia
set (DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)
set (DOXYGEN_QHP_NAMESPACE "org.deepin.dde.${CMAKE_PROJECT_NAME}")
set (DOXYGEN_QCH_FILE "${CMAKE_PROJECT_NAME}.qch")
set (DOXYGEN_QHP_VIRTUAL_FOLDER ${CMAKE_PROJECT_NAME})
set (DOXYGEN_PREDEFINED
    "DMULTIMEDIA_BEGIN_NAMESPACE=namespace Dtk { namespace Multimedia {"
    "DMULTIMEDIA_END_NAMESPACE=}}"
    "DMULTIMEDIA_USE_NAMESPACE=using namespace Dtk::Multimedia;"
)
doxygen_add_docs (doxygen
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dtkmultimedia
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dtkmultimediawidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../docs/dtkmultimedia
    ${CMAKE_CURRENT_SOURCE_DIR}/../docs/dtkmultimediawidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dtkocr
    ${CMAKE_CURRENT_SOURCE_DIR}/../docs/dtkocr
    ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
    COMMENT "Generate documentation via Doxygen"
)

#OCR
# set (OCR_BIN_NAME dtkocr)
# set (DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/${OCR_BIN_NAME})
# set (DOXYGEN_QHP_NAMESPACE "org.deepin.dde.${OCR_BIN_NAME}")
# set (DOXYGEN_QCH_FILE "${OCR_BIN_NAME}.qch")
# set (DOXYGEN_QHP_VIRTUAL_FOLDER ${OCR_BIN_NAME})
# set (DOXYGEN_PREDEFINED
#     "DOCR_BEGIN_NAMESPACE=namespace Dtk { namespace Ocr {"
#     "DOCR_END_NAMESPACE=}}"
#     "DOCR_USE_NAMESPACE=using namespace DOCR_NAMESPACE;"
# )


install (FILES ${CMAKE_CURRENT_BINARY_DIR}/html/${CMAKE_PROJECT_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
# install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${OCR_BIN_NAME}/html/${OCR_BIN_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
