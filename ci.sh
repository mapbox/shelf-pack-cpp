#!/usr/bin/env bash

case $CIRCLE_NODE_INDEX in
    0)
    export CXX="g++-4.9" CC="gcc-4.9"
    ;;
    *)
    export CXX="clang++" CC="clang++"
    ;;
esac

echo "$CXX"
echo `$CXX --version`
make clean
make
