cmake_minimum_required(VERSION 3.4)
project(qdata)

set(PROJECT_SOURCES
        images.c
        models.c
        qdata.c
        sprites.c
        tables.c
        video.c)

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