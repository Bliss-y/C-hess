@echo off
pushd build
cl -ZI ../test/hashmaptest.cpp Ws2_32.lib
popd
