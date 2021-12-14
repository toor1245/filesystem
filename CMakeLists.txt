cmake_minimum_required(VERSION 3.17)
project(filesystem C)

set(CMAKE_C_STANDARD 99)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(GNUInstallDirs)

macro(setup_include_and_definitions TARGET_NAME)
    target_include_directories(${TARGET_NAME}
            PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/internal>)
endmacro()

macro(add_fs_headers_and_sources HDRS_LIST_NAME SRCS_LIST_NAME)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/filesystem.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/file.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/descriptor.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/bitmap.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/linked_list.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/binary_tree.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/array_list.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/bit_utils.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/internal/filesystem_macros.h)
    list(APPEND ${HDRS_LIST_NAME} ${PROJECT_SOURCE_DIR}/include/command_line_parser.h)

    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/filesystem.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/array_list.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/file.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/bitmap.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/binary_tree.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/descriptor.c)
    list(APPEND ${SRCS_LIST_NAME} ${PROJECT_SOURCE_DIR}/src/command_line_parser.c)
endmacro()

set (FS_HDRS)
set (FS_SRCS)
add_fs_headers_and_sources(FS_HDRS FS_SRCS)
add_library(fs_lib ${FS_HDRS} ${FS_SRCS})
set_target_properties(fs_lib PROPERTIES PUBLIC_HEADER "${FS_HDRS}")
setup_include_and_definitions(fs_lib)
target_link_libraries(fs_lib PUBLIC ${CMAKE_DL_LIBS})
target_include_directories(fs_lib
        PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/fs_lib>
        )
add_library(filesystem::fs_lib ALIAS fs_lib)

#
# program : filesystem_demo
#

add_executable(filesystem_demo ${PROJECT_SOURCE_DIR}/src/main.c)
target_link_libraries(filesystem_demo PRIVATE fs_lib)
add_executable(filesystem::filesystem_demo ALIAS filesystem_demo)