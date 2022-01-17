project(lexer)

add_library(muaLexer lexer.cpp token_stream.cpp)
target_include_directories(muaLexer
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(muaLexer common muaTypes)