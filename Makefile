all:
	g++ -std=c++11 -o ss smain.cpp Socket.cpp
	g++ -std=c++11 -o sc cmain.cpp Socket.cpp -lpthread
	cp sc c1/
	cp sc c2/
	cp sc c3/
s:
	g++ -std=c++11 -o ss smain.cpp Socket.cpp
c:
	g++ -std=c++11 -o sc cmain.cpp Socket.cpp -lpthread
