project(types)

add_library(muaTypes magic_type_ext.cpp primitive_types.cpp list.cpp)
target_include_directories(muaTypes
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(muaTypes common)