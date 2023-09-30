@echo off
pushd build
cl -Zi ../src/%1.cpp Ws2_32.lib
popd
