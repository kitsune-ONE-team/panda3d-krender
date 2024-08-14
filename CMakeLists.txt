cmake_minimum_required(VERSION 3.10)
project(panda3d-krender)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../../..")

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions("/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 /GR-")
else()
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -fno-rtti")
endif()

if(EXISTS "${PYTHON_LIBRARY}")
else()
  find_package(PythonInterp 3.2 REQUIRED)
  find_package(PythonLibs 3.2 REQUIRED)
endif()
include_directories(${PYTHON_INCLUDE_DIR})

include_directories(${PANDA_INCLUDE_DIR}/panda3d)
link_directories(${PANDA_LIBRARY_DIR})

if(WIN32)
  if(EXISTS "${PANDA_LIBRARY_DIR}/libp3framework.lib")
    set(PANDA_PREFIX "lib")
  else()
    set(PANDA_PREFIX "")
  endif()
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
)

if(WIN32)
  if(EXISTS "${PANDA_LIBRARY_DIR}/libp3rplight.lib")
    set(PANDA_LIBS
      ${PANDA_LIBS}
      libp3rplight
    )
  else()
    set(PANDA_LIBS
      ${PANDA_LIBS}
      p3rplight
    )
  endif()
else()
  set(PANDA_LIBS
    ${PANDA_LIBS}
    p3rplight
  )
endif()

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(krender)

if(${WITH_TESTS} MATCHES "ON")
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/krender_tests.h.cpp
    COMMAND cxxtestgen --error-printer -o
    ${CMAKE_CURRENT_BINARY_DIR}/krender_tests.h.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/krender_tests.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/krender_tests.h)
  add_executable(krender_tests ${CMAKE_CURRENT_BINARY_DIR}/krender_tests.h.cpp)
  target_link_libraries(krender_tests core)

  add_custom_command(
    OUTPUT krender_tests__done
    COMMAND env LD_LIBRARY_PATH=${PANDA_LIBRARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/krender_tests
    DEPENDS krender_tests)
  add_custom_target(
    krender_tests__run ALL
    DEPENDS krender_tests__done)
endif()
