
all:	server.o client.o

server.o:	Server.cpp Utility.h SDES.h
	g++ Server.cpp -o server -std=c++11

client.o:	Client.cpp Utility.h SDES.h
	g++ Client.cpp -o client -std=c++11

clean:
	rm -f server.o client.o

