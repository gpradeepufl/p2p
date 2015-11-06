#include <iostream>
#include <stdio.h>
#include <vector>
#include "Socket.h"
#include "SocketException.h"
#include <string.h>

using namespace std;

int GetFileSize(std::string filename)
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
}

void build_fchunks(vector <char *> &fchunks, std::string fname)
{
	FILE *infp = fopen("in.txt", "r" );
	std::cout << "running....\n";
	size_t fsize = GetFileSize("in.txt");
	char *buf = new char[fsize];
	cout<< "Reported File size: "<<fsize<<endl;
	cout<<"No. of chunks: " << fsize/MAXRECV <<endl;
	memset (buf, 0, fsize + 1);
	size_t read = fread(buf, 1, fsize, infp);

	if (read != fsize)
		cout<< "ERROR: read and actual read"<<read << ", fsize" <<fsize<<endl; 
	for (int i = 0; i < fsize;) {
		fchunks.push_back(&buf[i]);
		i += (MAXRECV);
	}
	fclose(infp);
	return;
}

int main ( int argc, int argv[] )
{
	vector <char *> fchunks;
	build_fchunks (fchunks, "in.txt");
	char ctrl[MAXRECV];
	vector <int> sent;
	memset (ctrl, 0, MAXRECV + 1);
	try {
		// Create the socket
		Socket *serverSkt = new Socket(true, "", 20000);
		Socket *bind_sock = new Socket;
		serverSkt->accept(*bind_sock);
		std::cout << "Connected to Client\n";
		string nchunks(to_string(fchunks.size()));
		bind_sock->send(nchunks);
		try {
			while ( true ) {
				bind_sock->recv(ctrl);
				if (sent.size() == fchunks.size())
					break;
				int action = atoi(ctrl);
				bind_sock->send(fchunks[action], MAXRECV);
				sent.push_back(action);
			}
		} catch ( SocketException& ) {}
	} catch ( SocketException& e ) {
		std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
	}
cout<< "Sending Done \n";
	delete fchunks[0];
	return 0;
}
