cmake_minimum_required(VERSION 3.4...4.1.2)
project(pak)

set(PROJECT_SOURCES pak.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)

    if (CMAKE_BUILD_TYPE MATCHES RELEASE)
        target_compile_options(${PROJECT_NAME} PUBLIC -O3 -fomit-frame-pointer)
    endif ()
endif ()
