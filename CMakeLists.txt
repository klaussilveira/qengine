cmake_minimum_required(VERSION 3.4)
project(qengine VERSION 0.1.0)

option(BUILD_TOOLS "Build tools" ON)
option(BUILD_SDL "Build SDL platform" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_GL_RENDERER "Build OpenGL renderer instead of software renderer" OFF)

add_definitions(-DENGINE_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -fno-strict-aliasing -fwrapv -fcommon")
string(REPLACE "-O3"
               "-O2"
               CMAKE_C_FLAGS_RELEASE
               "${CMAKE_C_FLAGS_RELEASE}")

# Variables
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(PROJECT_INCLUDE_DIRS)
set(PROJECT_LINK_DIRS)
set(COMMON_LINKER_FLAGS)
set(CLIENT_LINKER_FLAGS)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND COMMON_LINKER_FLAGS "-lm -static-libgcc")
else()
  list(APPEND COMMON_LINKER_FLAGS "-lm -rdynamic")
endif()

# If we're building with gcc for i386 let's define -ffloat-store. This helps the old and crappy x87 FPU to produce
# correct values. Would be nice if Clang had something comparable.
if("${ARCH}" STREQUAL "i386")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffloat-store")
  endif()
endif()

# Force SSE math on x86_64. All sane compilers should do this anyway, just to protect us from broken Linux distros.
if("${ARCH}" STREQUAL "x86_64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpmath=sse")
endif()

# OpenGL renderer support
if(BUILD_GL_RENDERER)
  find_package(OpenGL REQUIRED)
  message(STATUS "Building with both software and OpenGL renderers")
else()
  message(STATUS "Building with software renderer only")
endif()

set(SERVER_PLATFORM_SOURCE ${SOURCE_DIR}/platform/generic/misc.c)

set(CLIENT_PLATFORM_SOURCE ${SOURCE_DIR}/platform/generic/misc.c ${SOURCE_DIR}/platform/generic/vid.c)

set(CLIENT_PLATFORM_HEADER
    ${SOURCE_DIR}/platform/graphics.h
    ${SOURCE_DIR}/platform/input.h
    ${SOURCE_DIR}/platform/sound.h)

set(SDL_SOURCE
    ${SOURCE_DIR}/platform/sdl/input.c
    ${SOURCE_DIR}/platform/sdl/sound.c)

set(SDL_SOURCE_SW ${SDL_SOURCE} ${SOURCE_DIR}/platform/sdl/graphics.c)
set(SDL_SOURCE_GL ${SDL_SOURCE} ${SOURCE_DIR}/platform/sdl/graphics_gl.c)

# Software renderer sources
set(RENDERER_SOURCE_SW
    ${SOURCE_DIR}/client/renderer/sw_aclip.c
    ${SOURCE_DIR}/client/renderer/sw_alias.c
    ${SOURCE_DIR}/client/renderer/sw_bsp.c
    ${SOURCE_DIR}/client/renderer/sw_draw.c
    ${SOURCE_DIR}/client/renderer/sw_edge.c
    ${SOURCE_DIR}/client/renderer/sw_image.c
    ${SOURCE_DIR}/client/renderer/sw_light.c
    ${SOURCE_DIR}/client/renderer/sw_main.c
    ${SOURCE_DIR}/client/renderer/sw_misc.c
    ${SOURCE_DIR}/client/renderer/sw_model.c
    ${SOURCE_DIR}/client/renderer/sw_part.c
    ${SOURCE_DIR}/client/renderer/sw_poly.c
    ${SOURCE_DIR}/client/renderer/sw_polyse.c
    ${SOURCE_DIR}/client/renderer/sw_rast.c
    ${SOURCE_DIR}/client/renderer/sw_scan.c
    ${SOURCE_DIR}/client/renderer/sw_sprite.c
    ${SOURCE_DIR}/client/renderer/sw_surf.c)

set(RENDERER_HEADER_SW
    ${SOURCE_DIR}/client/renderer/constants/adivtab.h
    ${SOURCE_DIR}/client/renderer/header/local.h
    ${SOURCE_DIR}/client/renderer/header/model.h
    ${SOURCE_DIR}/client/renderer/ref_shared.h)

# OpenGL renderer sources
set(RENDERER_SOURCE_GL
    ${SOURCE_DIR}/client/renderer/gl1/gl1_draw.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_image.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_light.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_lightmap.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_main.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_md2.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_mesh.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_misc.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_model.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_scrap.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_sdl.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_sp2.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_surf.c
    ${SOURCE_DIR}/client/renderer/gl1/gl1_warp.c
    ${SOURCE_DIR}/client/renderer/gl1/qgl.c
    ${SOURCE_DIR}/client/renderer/files/pcx.c
    ${SOURCE_DIR}/client/renderer/files/wal.c
    ${SOURCE_DIR}/client/renderer/files/stb.c
    ${SOURCE_DIR}/client/renderer/files/pvs.c)

set(RENDERER_HEADER_GL
    ${SOURCE_DIR}/client/renderer/gl1/header/local.h
    ${SOURCE_DIR}/client/renderer/gl1/header/model.h
    ${SOURCE_DIR}/client/renderer/gl1/header/qgl.h
    ${SOURCE_DIR}/client/renderer/ref_shared.h)

set(UNIX_SOURCE
    ${SOURCE_DIR}/platform/unix/main.c
    ${SOURCE_DIR}/platform/unix/network.c
    ${SOURCE_DIR}/platform/unix/signalhandler.c
    ${SOURCE_DIR}/platform/unix/system.c
    ${SOURCE_DIR}/platform/unix/memory.c)

set(UNIX_HEADER ${SOURCE_DIR}/platform/unix/header/unix.h)

set(WINDOWS_SOURCE
    ${SOURCE_DIR}/platform/windows/icon.rc
    ${SOURCE_DIR}/platform/windows/network.c
    ${SOURCE_DIR}/platform/windows/system.c
    ${SOURCE_DIR}/platform/windows/memory.c)

set(WINDOWS_HEADER ${SOURCE_DIR}/platform/windows/header/resource.h ${SOURCE_DIR}/platform/windows/header/winquake.h)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CLIENT_PLATFORM_SOURCE ${CLIENT_PLATFORM_SOURCE} ${WINDOWS_SOURCE})
  set(CLIENT_PLATFORM_HEADER ${CLIENT_PLATFORM_HEADER} ${WINDOWS_HEADER})

  set(SERVER_PLATFORM_SOURCE ${SERVER_PLATFORM_SOURCE} ${WINDOWS_SOURCE})
  set(SERVER_PLATFORM_HEADER ${SERVER_PLATFORM_HEADER} ${WINDOWS_HEADER})
else()
  set(CLIENT_PLATFORM_SOURCE ${CLIENT_PLATFORM_SOURCE} ${UNIX_SOURCE})
  set(CLIENT_PLATFORM_HEADER ${CLIENT_PLATFORM_HEADER} ${UNIX_HEADER})

  set(SERVER_PLATFORM_SOURCE ${SERVER_PLATFORM_SOURCE} ${UNIX_SOURCE})
  set(SERVER_PLATFORM_HEADER ${SERVER_PLATFORM_HEADER} ${UNIX_HEADER})
endif()

# Dependencies
if(BUILD_SDL)
  add_subdirectory(vendor/SDL2-2.0.22)

  list(APPEND PROJECT_INCLUDE_DIRS vendor/SDL2-2.0.22/include)
  list(APPEND COMMON_LINKER_FLAGS SDL2)
endif()

set(GAME_SOURCE
    ${SOURCE_DIR}/common/shared/flash.c
    ${SOURCE_DIR}/common/shared/rand.c
    ${SOURCE_DIR}/common/shared/shared.c
    ${SOURCE_DIR}/game/g_ai.c
    ${SOURCE_DIR}/game/g_chase.c
    ${SOURCE_DIR}/game/g_cmds.c
    ${SOURCE_DIR}/game/g_combat.c
    ${SOURCE_DIR}/game/g_func.c
    ${SOURCE_DIR}/game/g_items.c
    ${SOURCE_DIR}/game/g_main.c
    ${SOURCE_DIR}/game/g_misc.c
    ${SOURCE_DIR}/game/g_monster.c
    ${SOURCE_DIR}/game/g_phys.c
    ${SOURCE_DIR}/game/g_spawn.c
    ${SOURCE_DIR}/game/g_svcmds.c
    ${SOURCE_DIR}/game/g_target.c
    ${SOURCE_DIR}/game/g_trigger.c
    ${SOURCE_DIR}/game/g_utils.c
    ${SOURCE_DIR}/game/g_weapon.c
    ${SOURCE_DIR}/game/monster/infantry/infantry.c
    ${SOURCE_DIR}/game/monster/misc/move.c
    ${SOURCE_DIR}/game/monster/soldier/soldier.c
    ${SOURCE_DIR}/game/player/client.c
    ${SOURCE_DIR}/game/player/hud.c
    ${SOURCE_DIR}/game/player/trail.c
    ${SOURCE_DIR}/game/player/view.c
    ${SOURCE_DIR}/game/player/weapon.c)

set(GAME_HEADER
    ${SOURCE_DIR}/game/header/game.h
    ${SOURCE_DIR}/game/monster/infantry/infantry.h
    ${SOURCE_DIR}/game/monster/misc/player.h
    ${SOURCE_DIR}/game/monster/soldier/soldier.h)

set(CLIENT_SOURCE
    ${SOURCE_DIR}/client/cl_console.c
    ${SOURCE_DIR}/client/cl_effects.c
    ${SOURCE_DIR}/client/cl_entities.c
    ${SOURCE_DIR}/client/cl_input.c
    ${SOURCE_DIR}/client/cl_inventory.c
    ${SOURCE_DIR}/client/cl_keyboard.c
    ${SOURCE_DIR}/client/cl_lights.c
    ${SOURCE_DIR}/client/cl_main.c
    ${SOURCE_DIR}/client/cl_network.c
    ${SOURCE_DIR}/client/cl_parse.c
    ${SOURCE_DIR}/client/cl_particles.c
    ${SOURCE_DIR}/client/cl_prediction.c
    ${SOURCE_DIR}/client/cl_screen.c
    ${SOURCE_DIR}/client/cl_tempentities.c
    ${SOURCE_DIR}/client/cl_view.c
    ${SOURCE_DIR}/client/menu/menu.c
    ${SOURCE_DIR}/client/menu/qmenu.c
    ${SOURCE_DIR}/client/menu/videomenu.c
    ${SOURCE_DIR}/client/sound/sound.c
    ${SOURCE_DIR}/client/sound/wave.c
    ${SOURCE_DIR}/common/argproc.c
    ${SOURCE_DIR}/common/clientserver.c
    ${SOURCE_DIR}/common/collision.c
    ${SOURCE_DIR}/common/crc.c
    ${SOURCE_DIR}/common/cmdparser.c
    ${SOURCE_DIR}/common/cvar.c
    ${SOURCE_DIR}/common/filesystem.c
    ${SOURCE_DIR}/common/glob.c
    ${SOURCE_DIR}/common/md4.c
    ${SOURCE_DIR}/common/movemsg.c
    ${SOURCE_DIR}/common/frame.c
    ${SOURCE_DIR}/common/netchan.c
    ${SOURCE_DIR}/common/pmove.c
    ${SOURCE_DIR}/common/szone.c
    ${SOURCE_DIR}/common/zone.c
    ${SOURCE_DIR}/common/shared/flash.c
    ${SOURCE_DIR}/common/shared/rand.c
    ${SOURCE_DIR}/common/shared/shared.c
    ${SOURCE_DIR}/server/sv_cmd.c
    ${SOURCE_DIR}/server/sv_conless.c
    ${SOURCE_DIR}/server/sv_entities.c
    ${SOURCE_DIR}/server/sv_game.c
    ${SOURCE_DIR}/server/sv_init.c
    ${SOURCE_DIR}/server/sv_main.c
    ${SOURCE_DIR}/server/sv_send.c
    ${SOURCE_DIR}/server/sv_user.c
    ${SOURCE_DIR}/server/sv_world.c)

set(CLIENT_HEADER
    ${SOURCE_DIR}/client/header/client.h
    ${SOURCE_DIR}/client/header/console.h
    ${SOURCE_DIR}/client/header/keyboard.h
    ${SOURCE_DIR}/client/header/ref.h
    ${SOURCE_DIR}/client/header/screen.h
    ${SOURCE_DIR}/client/header/vid.h
    ${SOURCE_DIR}/client/menu/header/qmenu.h
    ${SOURCE_DIR}/client/sound/header/local.h
    ${SOURCE_DIR}/client/sound/header/sound.h
    ${SOURCE_DIR}/common/header/common.h
    ${SOURCE_DIR}/common/header/crc.h
    ${SOURCE_DIR}/common/header/files.h
    ${SOURCE_DIR}/common/header/glob.h
    ${SOURCE_DIR}/common/header/shared.h
    ${SOURCE_DIR}/common/header/zone.h
    ${SOURCE_DIR}/server/header/server.h)

set(SERVER_SOURCE
    ${SOURCE_DIR}/common/argproc.c
    ${SOURCE_DIR}/common/clientserver.c
    ${SOURCE_DIR}/common/collision.c
    ${SOURCE_DIR}/common/crc.c
    ${SOURCE_DIR}/common/cmdparser.c
    ${SOURCE_DIR}/common/cvar.c
    ${SOURCE_DIR}/common/filesystem.c
    ${SOURCE_DIR}/common/glob.c
    ${SOURCE_DIR}/common/md4.c
    ${SOURCE_DIR}/common/frame.c
    ${SOURCE_DIR}/common/movemsg.c
    ${SOURCE_DIR}/common/netchan.c
    ${SOURCE_DIR}/common/pmove.c
    ${SOURCE_DIR}/common/szone.c
    ${SOURCE_DIR}/common/zone.c
    ${SOURCE_DIR}/common/shared/rand.c
    ${SOURCE_DIR}/common/shared/shared.c
    ${SOURCE_DIR}/server/sv_cmd.c
    ${SOURCE_DIR}/server/sv_conless.c
    ${SOURCE_DIR}/server/sv_entities.c
    ${SOURCE_DIR}/server/sv_game.c
    ${SOURCE_DIR}/server/sv_init.c
    ${SOURCE_DIR}/server/sv_main.c
    ${SOURCE_DIR}/server/sv_send.c
    ${SOURCE_DIR}/server/sv_user.c
    ${SOURCE_DIR}/server/sv_world.c)

set(SERVER_HEADER
    ${SOURCE_DIR}/common/header/common.h
    ${SOURCE_DIR}/common/header/crc.h
    ${SOURCE_DIR}/common/header/files.h
    ${SOURCE_DIR}/common/header/glob.h
    ${SOURCE_DIR}/common/header/shared.h
    ${SOURCE_DIR}/common/header/zone.h
    ${SOURCE_DIR}/server/header/server.h)

include_directories(${PROJECT_INCLUDE_DIRS})
link_directories(${PROJECT_LINK_DIRS})

# Client executables
if(BUILD_GL_RENDERER)
  # Software renderer client
  add_executable(client-sw
                 ${CLIENT_PLATFORM_SOURCE}
                 ${CLIENT_PLATFORM_HEADER}
                 ${SDL_SOURCE_SW}
                 ${CLIENT_SOURCE}
                 ${CLIENT_HEADER}
                 ${RENDERER_SOURCE_SW}
                 ${RENDERER_HEADER_SW}
                 ${GAME_SOURCE}
                 ${GAME_HEADER})
  set_target_properties(client-sw PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  target_link_libraries(client-sw ${COMMON_LINKER_FLAGS} ${CLIENT_LINKER_FLAGS})

  # OpenGL renderer client
  add_executable(client-gl
                 ${CLIENT_PLATFORM_SOURCE}
                 ${CLIENT_PLATFORM_HEADER}
                 ${SDL_SOURCE_GL}
                 ${CLIENT_SOURCE}
                 ${CLIENT_HEADER}
                 ${RENDERER_SOURCE_GL}
                 ${RENDERER_HEADER_GL}
                 ${GAME_SOURCE}
                 ${GAME_HEADER})
  set_target_properties(client-gl PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  target_compile_definitions(client-gl PRIVATE USE_GL_RENDERER)
  target_link_libraries(client-gl ${COMMON_LINKER_FLAGS} ${CLIENT_LINKER_FLAGS} OpenGL::GL)
else()
  # Software renderer only
  add_executable(client-sw
                 ${CLIENT_PLATFORM_SOURCE}
                 ${CLIENT_PLATFORM_HEADER}
                 ${SDL_SOURCE_SW}
                 ${CLIENT_SOURCE}
                 ${CLIENT_HEADER}
                 ${RENDERER_SOURCE_SW}
                 ${RENDERER_HEADER_SW}
                 ${GAME_SOURCE}
                 ${GAME_HEADER})
  set_target_properties(client-sw PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  target_link_libraries(client-sw ${COMMON_LINKER_FLAGS} ${CLIENT_LINKER_FLAGS})
endif()

# Dedicated Server
add_executable(server
               ${SERVER_PLATFORM_SOURCE}
               ${SERVER_PLATFORM_HEADER}
               ${SERVER_SOURCE}
               ${SERVER_HEADER}
               ${GAME_SOURCE}
               ${GAME_HEADER})
set_target_properties(server
                      PROPERTIES COMPILE_DEFINITIONS
                                 "DEDICATED_ONLY"
                                 RUNTIME_OUTPUT_DIRECTORY
                                 ${CMAKE_BINARY_DIR})
target_link_libraries(server ${COMMON_LINKER_FLAGS})

if(BUILD_TOOLS)
  add_subdirectory(src/tools)
endif()

if(BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the docs.")
  endif()

  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  add_custom_target(docs ALL
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating docs"
                    VERBATIM)
endif()

# QA
add_custom_target(lint_check
  COMMAND find ${SOURCE_DIR} -iname *.h -o -iname *.c | xargs clang-format -output-replacements-xml | grep -c "<replacement " > /dev/null && echo "clang-format detected issues" && exit 1 || echo "No clang-format issues detected" && exit 0
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  VERBATIM
)

add_custom_target(lint
  COMMAND find ${SOURCE_DIR} -iname *.h -o -iname *.c | xargs clang-format -i
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  VERBATIM
)
