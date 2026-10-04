#!/usr/bin/env bash

if [ ! -f ./miku ]; then
    make
fi

./miku &
