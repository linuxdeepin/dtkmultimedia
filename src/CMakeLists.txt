add_subdirectory(multimedia)
add_subdirectory(multimediawidgets)
add_subdirectory(ocr)

# tablerecognizer 需要 onnxruntime；默认启用，可通过 -DDTK_ENABLE_TABLERECOGNIZER=OFF 跳过
# （如 CI 基础设施暂不可用 libonnxruntime-dev 时）。
option(DTK_ENABLE_TABLERECOGNIZER "Build tablerecognizer (requires onnxruntime)" ON)
if(DTK_ENABLE_TABLERECOGNIZER)
    add_subdirectory(tablerecognizer)
endif()
