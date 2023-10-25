cmake_minimum_required(VERSION 3.12)
project(omniGlass 
    VERSION 0.1.6
    DESCRIPTION "omni-glass touch gesture platform prototype"
)

find_path(LIBEVDEV_INCLUDE_DIR libevdev.h
    PATH_SUFFIXES "libevdev" "libevdev-1.0/libevdev")

add_library(platform_linux STATIC "src/platform_linux.c")
add_library(omniglass_linux SHARED "src/omniglass.c")

#target_include_directories(platform_linux 
#    PRIVATE $LIBEVDEV_INCLUDE_DIR)



#add_library(lua5.1 SHARED IMPORTED)
#add_library(evdev SHARED IMPORTED)
#find_package(lua 5.1)
#find_package(libevdev)
#find_library(lua lua5.1)

find_library(libevdev libevdev)
find_library(lua luajit)
add_executable(initialization_test "./src/tests/init.c")

# platform uses libevdev and lua 5.1
target_link_libraries(platform_linux evdev lua)

# resulting shared object uses the platform implementation
target_link_libraries(omniglass_linux platform_linux)
target_link_libraries(initialization_test platform_linux omniglass)
