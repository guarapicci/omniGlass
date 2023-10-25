#!/bin/sh

[ -e build ] || mkdir build
cp src/omniglass_linux.lua build/ &&\
gcc -fPIC -shared src/platform_linux.c src/omniglass.c -llua5.1 -levdev -o build/libomniglass.so &&\
gcc src/tests/init.c -L./build -lomniglass -o build/test.elf &&\
cd build &&\
sudo LD_LIBRARY_PATH=. ./test.elf
