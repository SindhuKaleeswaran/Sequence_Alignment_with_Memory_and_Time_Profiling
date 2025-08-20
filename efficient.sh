#!/bin/bash

g++ efficient.cpp -o efficient

./efficient "$1" "$2"
