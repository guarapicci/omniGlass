cmake_minimum_required(VERSION 3.12)
project(omniGlass 
    VERSION 0.1.6
    DESCRIPTION "omni-glass touch gesture platform prototype"
)

set_property(RUNTIME_OUTPUT_DIRECTORY = omniGlass_BINARY_DIR)

find_path(LIBEVDEV_INCLUDE_DIR libevdev.h
    PATH_SUFFIXES "libevdev" "libevdev-1.0/libevdev")

file(MAKE_DIRECTORY "output" "output/lib" "output/bin")
file(COPY_FILE "scripts/launcher.sh" "output/launcher.sh")

# dependencies: lua 5.1 and libevdev
find_library(evdev libevdev)
find_library(lua lua5.1)

add_library(platform_linux "src/platform_linux.c")
set_property(TARGET platform_linux
    PROPERTY POSITION_INDEPENDENT_CODE True
    RUNTIME_OUTPUT_DIRECTORY $<omniGlass_BINARY_DIR>)    
    
# platform uses libevdev and lua 5.1
target_link_libraries(platform_linux evdev lua)
target_include_directories(platform_linux PUBLIC "/usr/include/lua5.1/" "usr/include/lua51")    
add_library(omniglass_linux SHARED "src/omniglass.c")
target_link_libraries(omniglass_linux lua)

#target_include_directories(platform_linux 
#    PRIVATE $LIBEVDEV_INCLUDE_DIR)



#add_library(lua5.1 SHARED IMPORTED)
#add_library(evdev SHARED IMPORTED)
#find_package(lua 5.1)
#find_package(libevdev)

add_executable(initialization_test "./src/tests/init.c")



# resulting shared object uses the platform implementation
target_link_libraries(omniglass_linux platform_linux)
target_link_libraries(initialization_test omniglass_linux)
