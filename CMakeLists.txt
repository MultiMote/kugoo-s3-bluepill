cmake_minimum_required(VERSION 3.13)

option(SWAP_BEEP_AND_SPEED "Swap speed and beep buttons" OFF)
option(WARNINGS_AS_ERRORS "Threeat warnings as errors" OFF)
option(USE_DEFAULT_TOOLCHAIN "Use ./extra/arm-gcc-toolchain.cmake toolcahin" ON)

if(USE_DEFAULT_TOOLCHAIN)
  set(CMAKE_TOOLCHAIN_FILE ./extra/arm-gcc-toolchain.cmake)
endif()

project(kugoo-s3-bluepill VERSION 0.1)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Build type" FORCE)
endif()

set(TARGET_BASENAME ${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_BUILD_TYPE})
set(TARGET ${TARGET_BASENAME}.elf)

find_library(OPENCM3_LIBRARY opencm3_stm32f1
  HINTS
    ENV OPENCM3_ROOT
    ./libopencm3/lib
  PATH_SUFFIXES lib
  REQUIRED)

find_path(OPENCM3_INCLUDE_DIR libopencm3/stm32/gpio.h
  HINTS
    ENV OPENCM3_ROOT
    ./libopencm3
  PATH_SUFFIXES include
  REQUIRED)

find_program(OPENOCD openocd)

add_definitions(-DSTM32F1)

if(SWAP_BEEP_AND_SPEED)
  add_definitions(-DSWAP_BEEP_AND_SPEED)
endif()

add_definitions(-DFIRMWARE_VERSION="${PROJECT_VERSION}")

set(CMAKE_EXE_LINKER_FLAGS
"-T${PROJECT_SOURCE_DIR}/extra/stm32f103c8t6-opencm3.ld \
-mthumb \
-mcpu=cortex-m3 \
-nostartfiles \
-Wl,--gc-sections \
-Wl,-Map=${TARGET_BASENAME}.map")

# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-u _printf_float")

set(CMAKE_C_FLAGS
"-Wall \
-Wextra \
-std=gnu99 \
-mthumb \
-mcpu=cortex-m3 \
-mfloat-abi=soft \
-ffunction-sections \
-finline-functions \
-fdata-sections \
-specs=nano.specs \
-specs=nosys.specs")

if(WARNINGS_AS_ERRORS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()

include_directories(src)
include_directories(${OPENCM3_INCLUDE_DIR})

add_executable(${TARGET}
src/globals.c
src/utils.c
src/persistence.c
src/hardware.c
src/kugoo_s3.c
src/ssd1306.c
src/keyboard.c
src/gui.c

src/views/main_view.c
src/views/settings_view.c
src/views/detailed_view.c
src/views/trigger_calibration_view.c
src/views/last_trips_view.c

src/main.c)

target_link_libraries(${TARGET} ${OPENCM3_LIBRARY})

add_custom_command(
  TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET} ${TARGET_BASENAME}.hex
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

if(CMAKE_SIZE)
  add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley ${TARGET}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
else()
  message(WARNING "arm-none-eabi-size not found, please set the CMAKE_SIZE variable")
endif()

if (OPENOCD)
  add_custom_target(flash-openocd
    COMMAND ${OPENOCD} -d0 -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "program ${TARGET} verify reset exit"
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endif()

