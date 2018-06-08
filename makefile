CFLAGS=-std=c++17 -Wall -O2

%.o: %.cpp
	g++ $(CFLAGS) -c -o $@ $<

COMMON_OBJS= src/io/Reactor.o src/io/StdoutWriter.o src/utils/logging.o src/utils/errors.o src/utils/string.o src/radio/Receiver.o src/radio/Transmitter.o src/radio/IncomingAudioBuffer.o src/radio/protocol.o src/net/TextUDPSocket.o src/net/net.o src/net/TelnetServer.o src/net/UDPSocket.o

nadajnik: $(COMMON_OBJS) src/radio/nadajnik_main.o
	g++ $(CFLAGS) $(COMMON_OBJS) src/radio/nadajnik_main.o -o nadajnik

odbiornik: $(COMMON_OBJS) src/radio/odbiornik_main.o
	g++ $(CFLAGS) $(COMMON_OBJS) src/radio/odbiornik_main.o -o odbiornik

all: nadajnik odbiornik

clean:
	rm *.o nadajnik odbiornik
