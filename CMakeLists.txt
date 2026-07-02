cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(BatteryAssist)

set(SOURCES main.c)

# Create PRX module normally
add_prx_module(BatteryAssist ${SOURCES} exports.exp)

# Force the PSP SDK to configure this target as a Kernel module
set_target_properties(BatteryAssist PROPERTIES BUILD_WITH_KERNEL ON)

target_compile_options(BatteryAssist PRIVATE -O2 -Wall)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

# Link using the unified kernel interface library
target_link_libraries(BatteryAssist PRIVATE
    pspgu
    pspdisplay
    pspctrl
    pspge
    psppower
    pspkernel
)