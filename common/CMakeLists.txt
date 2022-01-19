# shared libs & pch

add_library(common common.cpp pch.h magic_type.hpp string_view_ext.hpp ref_ptr.hpp)
target_include_directories(common
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_precompile_headers(common
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)