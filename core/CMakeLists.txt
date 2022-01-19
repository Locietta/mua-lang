project(core)

add_library(muaCore parser.cpp)
target_include_directories(muaCore
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(muaCore common muaLexer)