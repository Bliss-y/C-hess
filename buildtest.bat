@echo off
pushd build
cl -ZI ../test/%1 Ws2_32.lib
popd
