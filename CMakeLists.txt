cmake_minimum_required(VERSION 3.13)
project(zelda3)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/SDL2" EXCLUDE_FROM_ALL)

set(SOURCE_FILES
        ancilla.cpp
        attract.cpp
        dungeon.cpp
        ending.cpp
        hud.cpp
        load_gfx.cpp
        main.cpp
        messaging.cpp
        misc.cpp
        nmi.cpp
        other_modules.cpp
        overlord.cpp
        overworld.cpp
        player.cpp
        player_oam.cpp
        poly.cpp
        select_file.cpp
        spc_player.cpp
        sprite.cpp
        sprite_main.cpp
        tagalong.cpp
        tile_detect.cpp
        tracing.cpp
        zelda_cpu_infra.cpp
        zelda_rtl.cpp
        snes/apu.cpp
        snes/cart.cpp
        snes/cpu.cpp
        snes/dma.cpp
        snes/dsp.cpp
        snes/input.cpp
        snes/ppu.cpp
        snes/snes.cpp
        snes/snes_other.cpp
        snes/spc.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2-static)