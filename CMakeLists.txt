cmake_minimum_required(VERSION 3.10)

project(cdebug)

aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/src" LIB_SRCS)

add_library(${PROJECT_NAME} STATIC ${LIB_SRCS})

target_compile_options(${PROJECT_NAME}
    PUBLIC -rdynamic -finstrument-functions -g -O0  -no-pie 
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/wrap
)

add_executable(trace ${CMAKE_CURRENT_LIST_DIR}/test/test-trace.c)
target_link_libraries(trace ${PROJECT_NAME})
