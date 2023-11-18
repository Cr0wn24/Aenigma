#!/bin/bash

mkdir -p ../build/debug
pushd ../build/debug


gcc -O2 -g -Werror -mavx2 -fpic -shared /home/hampus/Programmering/Aenigma/src/aenigma.c -olibgame.so -Wno-unused-result -lm

gcc -O2 -g /home/hampus/Programmering/Aenigma/src/linux_main.c -oaenigma_app -lX11 -ldl -lm

popd