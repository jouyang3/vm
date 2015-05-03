#!/bin/sh
make clean
rm -f *.so
make apps
make all
