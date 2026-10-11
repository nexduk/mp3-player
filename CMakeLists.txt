# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
set(USERHOME $ENV{USERPROFILE})
else()
set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_LIST_DIR}/libraries/pico-extras)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)



project(MP3-PLAYER C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ---- Third-party libraries -------------------------------------------------
add_subdirectory(libraries/FatFs_SPI)
add_subdirectory(libraries/pico_audio_i2s_32b/src/pico_audio_32b pico_audio_32b)
add_subdirectory(libraries/pico_audio_i2s_32b pico_audio_i2s_32b)

# ---- Application -----------------------------------------------------------
# One line per module. Add display.c / inputs.c here as you build them.
add_executable(MP3-PLAYER
    main.c
    hw_config.c
    src/audio_out.c
    src/storage.c
    src/mp3_player.c
)

set_source_files_properties(src/mp3_player.c PROPERTIES COMPILE_OPTIONS "-O3")

pico_set_program_name(MP3-PLAYER "MP3-PLAYER")
pico_set_program_version(MP3-PLAYER "0.1")

pico_enable_stdio_uart(MP3-PLAYER 0)
pico_enable_stdio_usb(MP3-PLAYER 1)

target_include_directories(MP3-PLAYER PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}                    # config.h
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/libraries/minimp3
)

target_link_libraries(MP3-PLAYER
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_dma
    hardware_pio
    hardware_vreg
    FatFs_SPI
    pico_audio_32b
    pico_audio_i2s_32b
)

pico_add_extra_outputs(MP3-PLAYER)

target_compile_definitions(MP3-PLAYER PRIVATE
    PICO_AUDIO_I2S_DMA_IRQ=1
)