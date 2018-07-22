cmake_minimum_required(VERSION 3.4)
project(tools)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/tools)

set(COMMON_SOURCES
        ../common/bmplib.c
        ../common/bspfile.c
        ../common/cmdlib.c
        ../common/l3dslib.c
        ../common/lbmlib.c
        ../common/mathlib.c
        ../common/mdfour.c
        ../common/polylib.c
        ../common/scriplib.c
        ../common/threads.c
        ../common/trilib.c
        ../common/wadlib.c)

set(COMMON_INCLUDE_DIRS ../common)

add_subdirectory(bspinfo)
add_subdirectory(qbsp3)
add_subdirectory(qdata)
add_subdirectory(qrad3)
add_subdirectory(qvis3)
add_subdirectory(pcx2wal)
add_subdirectory(pcx2pal)
add_subdirectory(16to8)
add_subdirectory(colormap)