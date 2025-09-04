cmake_minimum_required(VERSION 3.10)

project(test)

aux_source_directory(./  SRCS)

add_executable(${PROJECT_NAME} ${SRCS}) 

find_package(cdebug)
if (cdebug_FOUND)
    message("cdebug found success!")
else()
    message(FATAL_ERROR "Can't find cdebug CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")
endif()

target_link_libraries(${PROJECT_NAME} cdebug)
