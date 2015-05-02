#!/bin/sh
make clean
rm -f *.so
make apps
make all
printf "set args $@\nhandle SIGUSR1 SIGUSR2 nostop noprint\nrun" > gdbscript
gdb ./vm < gdbscript
rm -f gdbscript
