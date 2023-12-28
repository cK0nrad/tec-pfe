#!/bin/bash

BINARY="its"


set -e
g++ ./src/main.cpp `fltk-config --cxxflags --ldflags` -o its
./its