cmake_minimum_required(VERSION 3.10)
project(panda3d-krender)

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions("/wd4996 /wd4275 /wd4267 /wd4101 /wd4273")
else()
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
endif()

if(DEFINED PYTHON_LIBRARY)
else()
  find_package(PythonInterp 3.2 REQUIRED)
  find_package(PythonLibs 3.2 REQUIRED)
endif()
include_directories(${PYTHON_INCLUDE_DIR})

if(DEFINED PANDA_LIBRARY_DIR)
else()
  find_path(PANDA_INCLUDE_DIR NAMES panda3d/pandaSystem.h)
  find_library(PANDAFRAMEWORK_LIBRARY_PATH NAMES libp3framework.so libp3framework.lib p3framework.lib)
  get_filename_component(PANDA_LIBRARY_DIR ${PANDAFRAMEWORK_LIBRARY_PATH} DIRECTORY)
  link_directories(${PANDA_LIBRARY_DIR})
endif()
include_directories(${PANDA_INCLUDE_DIR}/panda3d)

if((EXISTS "${PANDA_LIBRARY_DIR}/libpandabullet.so") OR (EXISTS "${PANDA_LIBRARY_DIR}/libpandabullet.lib"))
  # makepanda build
  if(WIN32)
    set(PANDA_PREFIX "lib")
  else()
    set(PANDA_PREFIX "")
  endif()
  set(PANDA_LIBS
    # PRIMARY
    ${PANDA_PREFIX}p3framework
    # COMMON_PANDA_LIBS
    ${PANDA_PREFIX}panda
    ${PANDA_PREFIX}pandaexpress
    # COMMON_DTOOL_LIBS
    ${PANDA_PREFIX}p3dtool
    ${PANDA_PREFIX}p3dtoolconfig
    # EXTRA
    ${PANDA_PREFIX}p3direct
    ${PANDA_PREFIX}p3interrogatedb
    # RPLIGHT
    ${PANDA_PREFIX}p3rplight)

else()
  # cmake build
  set(PANDA_LIBS
    # PRIMARY
    p3framework
    # COMMON_PANDA_LIBS
    panda
    pandaexpress
    # COMMON_DTOOL_LIBS
    p3dtool
    p3dtoolconfig
    # EXTRA
    p3direct
    p3interrogatedb
    # RPLIGHT
    p3rplight)
endif()

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(krender)
