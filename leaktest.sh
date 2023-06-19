#!/usr/bin/env bash

make clean
make debug
valgrind --leak-check=full ./build/linux/game_linux