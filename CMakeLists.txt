cmake_minimum_required(VERSION 3.8)
project(Mojave_Radio)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_definitions(-DDEBUG)

set(SOURCES
        src/io/Reactor.cpp src/io/Reactor.h
        src/utils/chrono.h
        src/utils/functional.h
        src/io/StdinReader.cpp src/io/StdinReader.h
        src/io/io.h
        src/net/TelnetServer.cpp src/net/TelnetServer.h
        src/net/UDPSocket.cpp src/net/UDPSocket.h
        src/utils/errors.h src/utils/errors.cpp
        src/net/TextUDPSocket.cpp src/net/TextUDPSocket.h
        src/net/net.h src/net/net.cpp
        src/utils/constants.h
        src/utils/FIFOBuffer.h
        src/radio/protocol.cpp src/radio/protocol.h
        src/utils/logging.h src/utils/logging.cpp
        src/utils/string.h src/utils/string.cpp
        src/radio/Receiver.cpp src/radio/Receiver.h
        src/radio/Transmitter.cpp src/radio/Transmitter.h src/io/StdoutWriter.cpp src/io/StdoutWriter.h src/utils/HollowBuffer.cpp src/utils/HollowBuffer.h)

# add_executable(Playground src/radio/main.cpp ${SOURCES})

add_executable(odbiornik
        ${SOURCES} src/radio/odbiornik_main.cpp)

add_executable(nadajnik
        ${SOURCES} src/radio/nadajnik_main.cpp)

set(LIBRARIES
        boost_program_options)

#TARGET_LINK_LIBRARIES(Playground LINK_PUBLIC ${LIBRARIES})
TARGET_LINK_LIBRARIES(odbiornik LINK_PUBLIC ${LIBRARIES})
TARGET_LINK_LIBRARIES(nadajnik LINK_PUBLIC ${LIBRARIES})