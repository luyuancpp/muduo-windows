cmake_minimum_required(VERSION 3.2.0)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(muduo C CXX)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if (MSVC)
  # Build with multiple processes
  add_definitions(/MP)
  add_definitions( /wd4068 /wd4996 /wd4244 /wd4820)
  # Allow big object
  add_definitions(/bigobj)
  
else (MSVC)

  set(CXX_FLAGS
  -g
  # -DVALGRIND
  # -DMUDUO_STD_STRING
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wno-unused-parameter
  #-Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
  -march=native
  -rdynamic
  -std=c++20
  -pthread 
  -lpthread 
  --std=c++20 
  -D_GLIBCXX_USE_DEPRECATED=0
  )
  
if(CMAKE_BUILD_BITS EQUAL 64)
  list(APPEND CXX_FLAGS "-m64")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
endif (MSVC)

add_definitions(-DGOOGLE_CHECK_EQ=ABSL_CHECK_EQ)
add_definitions(-DGOOGLE_LOG=ABSL_LOG)
add_definitions(-DGOOGLE_DCHECK=ABSL_DCHECK)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DNOMINMAX)
add_definitions(-DABSL_PROPAGATE_CXX_STD=TRUE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(third_party/boost)
include_directories(contrib/windows/include)
include_directories(third_party/protobuf/third_party/googletest/googletest/include)
include_directories(third_party/protobuf/third_party/googletest/googlemock/include)
include_directories(third_party/protobuf/src)
include_directories(third_party/protobuf/third_party/abseil-cpp)
include_directories(third_party/zlib)
include_directories(muduo/net/protorpc)
include_directories("${zlib_BINARY_DIR}")
include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

# Add subdirectories
add_subdirectory(third_party/zlib)
add_subdirectory(third_party/protobuf)
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
#add_subdirectory(examples/protobuf)
