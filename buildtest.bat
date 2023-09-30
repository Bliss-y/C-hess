@echo off
pushd build
cl -Zi ../test/%1.cpp Ws2_32.lib ../src/utils/parsehttp.cpp
popd
