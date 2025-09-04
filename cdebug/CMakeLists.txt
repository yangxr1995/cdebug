cmake_minimum_required(VERSION 3.10)

project(cdebug)

aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/src" LIB_SRCS)

add_library(${PROJECT_NAME} STATIC ${LIB_SRCS})

option(WRAP_ON "wrap func turn on" OFF)

if(WRAP_ON)
    include(${CMAKE_CURRENT_LIST_DIR}/wrap.cmake)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE WRAP_TURN_ON
    )
endif()

target_link_libraries(${PROJECT_NAME} dl pthread)

target_compile_options(${PROJECT_NAME}
    PUBLIC -finstrument-functions -g -O0 -funwind-tables
)

target_link_options(${PROJECT_NAME}
    PUBLIC -rdynamic
 -Wl,--wrap=fork 
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
)

install(TARGETS ${PROJECT_NAME}

    # 生成config信息
    EXPORT ${PROJECT_NAME}

    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# 安装config信息
install(EXPORT ${PROJECT_NAME}
    FILE ${PROJECT_NAME}Config.cmake 
    DESTINATION config
)
