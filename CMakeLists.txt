cmake_minimum_required(VERSION 3.12)
project(omniGlass 
    VERSION 0.1.0
    DESCRIPTION "omni-glass touch gesture platform prototype"
)

find_path(LIBEVDEV_INCLUDE_DIR libevdev.h
    PATH_SUFFIXES "libevdev" "libevdev-1.0/libevdev")

add_library(platform_linux STATIC "src/platform_linux.c")

#target_include_directories(platform_linux 
#    PRIVATE $LIBEVDEV_INCLUDE_DIR)



#add_library(lua5.1 SHARED IMPORTED)
#add_library(evdev SHARED IMPORTED)
#find_package(lua 5.1)
#find_package(libevdev)
#find_library(lua lua5.1)

find_library(libevdev libevdev)
find_library(lua luajit)
add_executable(dummy
    "./src/platform_linux.c"
)
target_link_libraries(dummy evdev lua)
