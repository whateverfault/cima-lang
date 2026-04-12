cmake_minimum_required(VERSION 3.31)
project(ast_calc C)

set(CMAKE_C_STANDARD 11)

add_executable(ast_calc 
        src/main.c
        src/lexer/lexer.c
        src/parser/parser.c
        src/executor/funcs.c
        src/types/type.c
        src/executor/executor.c
        src/other/sys/sys.c)

target_include_directories(ast_calc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty)
