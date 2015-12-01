#!/bin/bash

MYFLAGS="-std=c++1z -pthread -Wall -Wextra -Wpedantic -Wundef \
-Wno-missing-field-initializers -Wpointer-arith -Wcast-align -Wwrite-strings \
-Wno-unreachable-code -Wnon-virtual-dtor -Woverloaded-virtual -O0 -DSSVUT_DISABLE -g3 \
-Wno-unused-value" # <- To avoid writing `(void)` all over the place.

clang++ -o /tmp/$1 $MYFLAGS ./$1.cpp && /tmp/$1