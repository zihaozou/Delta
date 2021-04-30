include_directories(${PROJECT_SOURCE_DIR}/../MD5)
aux_source_directory(. CLASS_DIR)

add_library(class ${CLASS_DIR})
target_link_libraries(class PRIVATE md5)
