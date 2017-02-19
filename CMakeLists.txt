cmake_minimum_required(VERSION 2.8)

project(KiwiScheduler)

option(GCOV_SUPPORT "Build for gcov" Off)

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
set(CMAKE_MACOSX_RPATH ON)

if(NOT APPLE)
find_package(Threads)
endif()

if(UNIX)
  add_definitions("-std=c++11")
  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-weak-vtables -Wno-padded -Wno-float-equal")
  endif()
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

file(GLOB KIWI_SCHEDULER_SOURCES ${PROJECT_SOURCE_DIR}/sources/*.cpp ${PROJECT_SOURCE_DIR}/sources/*.hpp)
source_group(KiwiScheduler FILES ${KIWI_SCHEDULER_SOURCES})
include_directories(${PROJECT_SOURCE_DIR}/sources)

file(GLOB KIWI_SCHEDULER_TESTS ${PROJECT_SOURCE_DIR}/tests/*.cpp ${PROJECT_SOURCE_DIR}/tests/*.hpp)
source_group(Tests FILES ${KIWI_SCHEDULER_TESTS})
include_directories(${PROJECT_SOURCE_DIR}/tests)

add_executable(KiwiSchedulerTest ${KIWI_SCHEDULER_SOURCES} ${KIWI_SCHEDULER_TESTS})
if(NOT APPLE)
target_link_libraries(KiwiSchedulerTest Threads::Threads)
endif()

if(${GCOV_SUPPORT} STREQUAL "On")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage")
    target_link_libraries(KiwiSchedulerTest gcov)
endif()
