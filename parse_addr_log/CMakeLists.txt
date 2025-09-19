cmake_minimum_required(VERSION 3.12)

project(parse_addr_log)

add_executable(${PROJECT_NAME} main.cc)

target_compile_options(${PROJECT_NAME}
    PRIVATE -g
    PRIVATE -O0
    )
