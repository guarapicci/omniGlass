#!/bin/sh

# generate documentation
#doxygen doxcfg.txt

# build the library using gcc
[ -e build ] || mkdir build
cp src/omniglass_linux.lua build/ &&\
cp templates/config.lua build/ &&\
gcc -fPIC -shared src/platform_linux.c src/omniglass.c -I/usr/include/lua5.1 -L/usr/lib/lua5.1 -llua5.1 -I/usr/include/libevdev-1.0 -levdev -o build/libomniglass.so &&\
gcc src/tests/init.c -L./build -lomniglass -o build/test.elf &&\
cd build &&\
sudo LD_LIBRARY_PATH=. ./test.elf
