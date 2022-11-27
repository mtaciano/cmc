#!/bin/sh

# Modificar para o seu gosto
mkdir -p build
cd build
cmake -DCMAKE_C_COMPILER=clang \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold -Wl,-O1 -Wl,--as-needed" \
    -G "Ninja" ..

# Padrão
# mkdir -p build
# cd build
# cmake ..

# Mudar o tipo de build
# -DCMAKE_BUILD_TYPE=Debug
