cmake_minimum_required(VERSION 3.20)
project(mua-lang VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

add_subdirectory(common)
add_subdirectory(types)
add_subdirectory(lexer)
add_subdirectory(core)

add_executable(mua mua.cpp)
target_link_options(mua PRIVATE $<$<BOOL:${MINGW}>:-static-libgcc>)
target_link_libraries(mua muaCore)

add_custom_command(TARGET mua POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:mua> $<TARGET_FILE:mua> $<TARGET_FILE_DIR:mua>
    COMMAND_EXPAND_LISTS
)