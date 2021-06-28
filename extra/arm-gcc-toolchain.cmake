set(CMAKE_SYSTEM_NAME Generic)

# undefined reference to `_exit' fix on compiler test
set(CMAKE_EXE_LINKER_FLAGS_INIT "--specs=nosys.specs")

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_DEBUGGER     arm-none-eabi-gdb)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)
set(CMAKE_SIZE         arm-none-eabi-size)
