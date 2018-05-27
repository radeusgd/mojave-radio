cmake_minimum_required(VERSION 3.9)
project(Mojave_Radio)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(Mojave_Radio src/main.cpp src/io/Reactor.cpp src/io/Reactor.h src/chrono.h src/functional.h src/net/Socket.cpp src/net/Socket.h)