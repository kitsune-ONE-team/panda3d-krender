#!/bin/sh
# build module into SDK

SDK=/app/opt/sdk
PYTHON_VERSION=3.13t

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=${SDK}/lib/python${PYTHON_VERSION}/site-packages \
    -DINSTALL_PY=ON \
    -DINTERROGATE_EXECUTABLE=/app/opt/interrogate/bin/interrogate \
    -DINTERROGATE_MODULE_EXECUTABLE=/app/opt/interrogate/bin/interrogate_module \
    -DINTERROGATE_SOURCE_DIR=/app/jenkins/workspace/interrogate-lynx64 \
    -DMULTITHREADED_BUILD=16 \
    -DPANDA_INCLUDE_DIR=${SDK}/include \
    -DPANDA_LIBRARY_DIR=${SDK}/lib \
    -DPYTHON_EXECUTABLE=${SDK}/bin/python${PYTHON_VERSION} \
    -DPYTHON_INCLUDE_DIR=${SDK}/include/python${PYTHON_VERSION} \
    -DPYTHON_LIBRARY=${SDK}/lib/libpython${PYTHON_VERSION}.so.1.0 \
    -DWITH_TESTS=ON \
    ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
