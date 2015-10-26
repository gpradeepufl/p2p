# Makefile for the socket programming example
#

simple_server_objects = ServerSocket.o Socket.o smain.o
simple_client_objects = ClientSocket.o Socket.o cmain.o


all : simple_server simple_client

simple_server: $(simple_server_objects)
	g++ -o ss $(simple_server_objects)


simple_client: $(simple_client_objects)
	g++ -o sc $(simple_client_objects)


Socket: Socket.cpp
ServerSocket: ServerSocket.cpp
ClientSocket: ClientSocket.cpp
smain: smain.cpp
cmain: cmain.cpp


clean:
	rm -f *.o ss sc
