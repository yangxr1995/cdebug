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
    PUBLIC -rdynamic -no-pie
    PUBLIC -Wl,--wrap=fork 
)

# message("COMPILE_OPTIONS : ${INTERFACE_COMPILE_OPTIONS}")

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
)

install(TARGETS ${PROJECT_NAME}

    # 生成config信息
    EXPORT ${PROJECT_NAME}

    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# INTERFACE_LINK_LIBRARIES

# -Wl,--wrap=xxx
get_target_property(${PROJECT_NAME}_INTERFACE_LINK_OPTIONS ${PROJECT_NAME} INTERFACE_LINK_OPTIONS)
list(APPEND ${PROJECT_NAME}_INTERFACE_LINK_OPTIONS "-lpthread")
list(APPEND ${PROJECT_NAME}_INTERFACE_LINK_OPTIONS "-ldl")
string(REPLACE ";" "   " FORMATTED_COMPILE_OPTIONS "${${PROJECT_NAME}_INTERFACE_LINK_OPTIONS}")
set(${PROJECT_NAME}_LDFLAGS ${FORMATTED_COMPILE_OPTIONS})

get_target_property(${PROJECT_NAME}_INTERFACE_COMPILE_OPTIONS ${PROJECT_NAME} INTERFACE_COMPILE_OPTIONS)
string(REPLACE ";" "   " FORMATTED_LINK_OPTIONS "${${PROJECT_NAME}_INTERFACE_COMPILE_OPTIONS}")
set(${PROJECT_NAME}_CFLAGS ${FORMATTED_LINK_OPTIONS})

# 生成 Makefile 可用的配置文件（关键步骤）
configure_file(lib${PROJECT_NAME}.mk.in ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.mk @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.mk DESTINATION lib/cmake/lib${PROJECT_NAME})

# 可选：生成 pkg-config 文件（包含所有选项）
configure_file(lib${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

# 安装config信息
install(EXPORT ${PROJECT_NAME}
    FILE ${PROJECT_NAME}Config.cmake 
    DESTINATION lib/config
)
