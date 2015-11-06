#include "Socket.h"
#include "SocketException.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <string.h>

using namespace std;
struct common_data {
	string ofname;
	int upport, dwport;
	vector <char *> fchunks;
	vector <bool> is_fcpresent;
	int nChunksNeeded;
};

void *uploader(void *cdata) {
	char ctrl[MAXRECV];
	vector <int> sent;
	memset (ctrl, 0, MAXRECV + 1);
	struct common_data *cd = (struct common_data *) cdata;
 
	cout << "Uploader Thread created" << endl;

	try {
		// Create the socket
		Socket *serverSkt = new Socket(true, "", cd->upport);// ( 30000 );
		Socket *bind_sock = new Socket;
		serverSkt->accept(*bind_sock);
		std::cout << "[UPLOADER]: Waiting to connect to Client\n";
		bind_sock->recv(ctrl);
		int num_tosend = atoi(ctrl);
		try {
			while (sent.size() != num_tosend) {
				memset (ctrl, 0, MAXRECV + 1);
				bind_sock->recv(ctrl);
				int cid = atoi(ctrl);
				if (cd->is_fcpresent[cid]) {
					bind_sock->send(to_string(1));
					bind_sock->send(cd->fchunks[cid], MAXRECV);
					sent.push_back(cid);
				} else {
					bind_sock->send(to_string(0));
				}
			}
		} catch ( SocketException& ) {}
	} catch ( SocketException& e ) {
		std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
	}
	cout<< "Sending Done \n";
	pthread_exit(NULL);
}

void *downloader(void *cdata) {
	char ctrl[MAXRECV];
	vector <int> recvd;
	memset (ctrl, 0, MAXRECV + 1);
	struct common_data *cd = (struct common_data *) cdata;
	int n = cd->fchunks.size();

	cout << "DThread created" << endl;
	while (1) {
		try
		{
			Socket csocket ( false, "localhost", cd->dwport);
			csocket.send(to_string(cd->nChunksNeeded));
			int cid = 0;
			while (recvd.size() != cd->nChunksNeeded) {
				if (cid == n)
					cid = 0;
				if (cd->is_fcpresent[cid]) {cid++; continue;}

				csocket.send(to_string(cid));
				memset(ctrl, 0, MAXRECV + 1);
				csocket.recv(ctrl);
				if (!atoi(ctrl)) {cid++; continue;}

				csocket.recv(cd->fchunks[cid]);
				cid++;
			}
			break;
		} catch ( SocketException& e ) {
			cout << "[DOWNLOADER] waiting for uploader to join"<< "\n";
			sleep(5);
		} 
	}
	pthread_exit(NULL);
}

void build_fchunks(vector<char *> &fcs, vector <bool> is_fc, int n) {
	for (int i=0; i<n; i++) {
		char *newChunk = new char[MAXRECV];
		memset(newChunk, 0, MAXRECV+1);
		fcs.push_back(newChunk);
		is_fc.push_back(false);
	}
}

void destroy_fchunks(vector<char *> &fcs) {
	for (int i=0; i<fcs.size(); i++) {
		delete fcs[i];
	}
}

int main ()
{
	FILE *op = fopen("out.txt", "w" );
	int sock, rc;
	struct common_data cdata;
	pthread_t up_th, dw_th;
	cout<< "Enter Client id: ";
	cin >> sock;
	cout<< "Enter Upload Socket: ";
	cin >> cdata.upport;
	cout<< "Enter Download Socket: ";
	cin >> cdata.dwport;
// Phase - 1: Get chunksizes, filename from Server.
	try
	{
		Socket client_socket ( false, "localhost", 20000 + sock);
		cout<<"Getting Resp from server \n";
		char buf[MAXRECV + 1];
		string s;
		client_socket.recv(buf);
		int nchunks = atoi(buf);
		build_fchunks(cdata.fchunks, cdata.is_fcpresent, nchunks);
        	memset ( buf, 0, MAXRECV + 1 );		
		client_socket.recv(buf);
		cdata.ofname = buf;
		cout<<"File Name: " << cdata.ofname << " Chunk size: "<<nchunks<<endl;
        	memset ( buf, 0, MAXRECV + 1 );		
		client_socket.recv(buf);
		int num_chunks_iget = atoi(buf);
		cdata.nChunksNeeded = nchunks- num_chunks_iget;
		for (int i =0; i < num_chunks_iget; i++) {
			memset ( buf, 0, MAXRECV + 1 );		
			client_socket.recv(buf);
			int cid = atoi(buf);
			int rd = client_socket.recv(cdata.fchunks[cid]);
			cdata.is_fcpresent[cid] = true;
			cout << "Received CHUNK# "<<i << " "<< rd <<" Bytes"<<endl;
		}
	} catch ( SocketException& e ) {
		cout << "Exception was caught:" << e.description() << "\n";
	}
	//fclose(op);
	rc = pthread_create(&up_th, NULL, uploader, (void *)(&cdata));
      	if (rc) {
		cout << "Error:unable to create uploader thread," << rc << endl;
         	exit(-1);
      	}
	rc = pthread_create(&dw_th, NULL, downloader, (void *)(&cdata));
      	if (rc) {
		cout << "Error:unable to create downloader thread," << rc << endl;
         	exit(-1);
      	}
   	pthread_exit(NULL);
}
