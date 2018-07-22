cmake_minimum_required(VERSION 3.4)
project(16to8)

set(PROJECT_SOURCES 16to8.c)

add_executable(${PROJECT_NAME} ${COMMON_SOURCES} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDE_DIRS})

if (UNIX)
    set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} -lm)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_LINK_LIBRARIES})

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -pedantic -Wextra -fno-strict-aliasing)

    if (CMAKE_BUILD_TYPE MATCHES RELEASE)
        target_compile_options(${PROJECT_NAME} PUBLIC -O3 -fomit-frame-pointer)
    endif ()
endif ()