cmake_minimum_required(VERSION 3.9)
project(Mojave_Radio)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(Mojave_Radio
        src/main.cpp
        src/io/Reactor.cpp
        src/io/Reactor.h
        src/chrono.h
        src/functional.h
        src/io/StdinReader.cpp
        src/io/StdinReader.h
        src/io/io.h
        src/net/net.h src/net/TcpServer.cpp
        src/net/TcpServer.h
        src/net/MulticastSocket.cpp
        src/net/MulticastSocket.h src/errors.h src/errors.cpp src/net/TextMulticastSocket.cpp src/net/TextMulticastSocket.h src/net/net.cpp)