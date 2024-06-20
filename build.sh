#!/bin/sh

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=/panda3d-krender/lib/python3.11/site-packages \
    -DMULTITHREADED_BUILD=16 \
    -DINSTALL_PY=ON \
    -DPANDA_BINARY_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/bin \
    -DPANDA_INCLUDE_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/include \
    -DPANDA_LIBRARY_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/lib \
    -DPYTHON_INCLUDE_DIR=/root/jenkins/workspace/python-lynx64/dist/python/include/python3.11 \
    -DPYTHON_EXECUTABLE=/root/jenkins/workspace/python-lynx64/dist/python/bin/python3.11 \
    -DPYTHON_LIBRARY=/root/jenkins/workspace/python-lynx64/dist/python/lib/libpython3.11.so.1.0 \
    ..

make -j 16
make install DESTDIR=../dist/ -j 16

cd ..
