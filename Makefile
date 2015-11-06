all:
	g++ -std=c++11 -o ss smain.cpp Socket.cpp
	g++ -std=c++11 -o sc cmain.cpp Socket.cpp -lpthread
s:
	g++ -std=c++11 -o ss smain.cpp Socket.cpp
c:
	g++ -std=c++11 -o sc cmain.cpp Socket.cpp -lpthread
