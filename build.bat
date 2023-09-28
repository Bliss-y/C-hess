@echo off
pushd build
cl -ZI ../src/example.cpp Ws2_32.lib
popd
