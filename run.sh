#! /bin/sh -e

cmd="./bin/helloworld"
lib_path="./lib/bin/libevil.so"

make

printf "\nNormal:\n"
$cmd

printf "\nUsing LD_PRELOAD:\n"
LD_PRELOAD=$lib_path $cmd


