cmake_minimum_required(VERSION 3.12)
project(omniGlass 
    VERSION 0.1.6
    DESCRIPTION "omni-glass touch gesture platform prototype"
)

include(GNUInstallDirs)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


file(MAKE_DIRECTORY "output" "output/lib" "output/bin")

configure_file(src/omniglass_linux.lua bin/omniglass_linux.lua COPYONLY)
configure_file(src/omniglass_core.lua bin/omniglass_core.lua COPYONLY)
configure_file(templates/config.lua bin/config.lua COPYONLY)
configure_file(scripts/launcher.sh ${CMAKE_BINARY_DIR} COPYONLY)


# dependencies: lua 5.1 and libevdev
find_library(evdev libevdev)
find_library(luaruntime lua5.1)

# resulting shared object uses the platform implementation
add_library(omniglass_linux SHARED "src/omniglass.c" "src/platform_linux.c")
target_link_libraries(omniglass_linux lua5.1 evdev)
if(NOT DEFINED LUA_INCLUDE_DIR)
        set(LUA_INCLUDE_DIR "/usr/include/lua5.1")
endif()
if(NOT DEFINED LIBEVDEV_INCLUDE_DIR)
        set(LIBEVDEV_INCLUDE_DIR "/usr/include/libevdev-1.0")
endif()
target_include_directories(omniglass_linux PUBLIC ${LUA_INCLUDE_DIR} PUBLIC ${LIBEVDEV_INCLUDE_DIR})

# simple app, just runs init (underlying platform prints out various checks)
add_executable(initialization_test "src/tests/init.c")
target_link_libraries(initialization_test omniglass_linux lua5.1)

# demo showcasing left/right slide and tapping
add_executable(swipetap "src/tests/swipetap.c")
target_link_libraries(swipetap omniglass_linux)

# demo for bottom edge slide
add_executable(edge_swipe "src/tests/edge_swipe.c")
target_link_libraries(edge_swipe omniglass_linux)

# demo for multi edge slide
add_executable(edge_swipe_multi "src/tests/edge_swipe_multi.c")
target_link_libraries(edge_swipe_multi omniglass_linux)

install(TARGETS omniglass_linux 
        LIBRARY
            DESTINATION ${CUSTOM_LIB_FOLDER})
install(PROGRAMS "src/omniglass_core.lua" "src/omniglass_linux.lua"
        DESTINATION ${CUSTOM_LIB_FOLDER}/omniGlass)
install(PROGRAMS "templates/config.lua"
        DESTINATION ${CUSTOM_CONFIG_FOLDER}/omniGlass)
install(FILES "src/omniglass.h" "src/config.h" "src/constants.h"
        DESTINATION ${CUSTOM_INCLUDE_FOLDER}/omniGlass)
install(TARGETS edge_swipe_multi
        RUNTIME
            DESTINATION ${CUSTOM_EXEC_FOLDER}/omniGlass)
