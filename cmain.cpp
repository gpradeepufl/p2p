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
void make_final_file(struct common_data *cd) {
	FILE *outfp = fopen((cd->ofname).c_str(), "w" );
	for (int i=0; i < cd->fchunks.size(); i++) {
		fwrite (cd->fchunks[i], sizeof(char), MAXRECV, outfp);
		delete cd->fchunks[i];
	}
	fclose(outfp);
	return;
}
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
		bind_sock->send(to_string(1));
		int num_tosend = atoi(ctrl);
		std::cout << "[UPLOADER]: Client Connected: " <<num_tosend<<"\n";

		try {
			while (sent.size() != num_tosend) {
				memset (ctrl, 0, MAXRECV + 1);
				bind_sock->recv(ctrl);
				int cid = atoi(ctrl);
				cout << "[UPLOADER]: Client requested CID: " << cid;
				bind_sock->send(to_string(1));

				if (cd->is_fcpresent[cid]) {
					bind_sock->send(to_string(10));
					bind_sock->recv(ctrl);
					bind_sock->send(cd->fchunks[cid], MAXRECV);
					bind_sock->recv(ctrl);
					sent.push_back(cid);
					cout << "TRANSFERED \n";
				} else {
					bind_sock->send(to_string(11));
					bind_sock->recv(ctrl);
					cout << "NOT PRESENT YET \n";
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
			std::cout << "[DOWNLOADER]: Connected to SERVER\n" ;
			csocket.send(to_string(cd->nChunksNeeded));
			memset(ctrl, 0, MAXRECV + 1);
			csocket.recv(ctrl); // ACK
			int cid = 0;
			while (recvd.size() != cd->nChunksNeeded) {
				if (cid == n)
					cid = 0;
				if (cd->is_fcpresent[cid]) {cid++; continue;}

				csocket.send(to_string(cid)); // Send CID
				memset(ctrl, 0, MAXRECV + 1);
				csocket.recv(ctrl);// RECV ACK
				memset(ctrl, 0, MAXRECV + 1);
				csocket.recv(ctrl);// RECV IF_PRESENT
				csocket.send(to_string(1));
				if (atoi(ctrl) == 11) {cid++; continue;}
				// PRESENT - RECV DATA AND SEND ACK
				csocket.recv(cd->fchunks[cid]);
				csocket.send(to_string(1));
				cd->is_fcpresent[cid] = true;
				recvd.push_back(cid);
				cid++;
			}
			cout << "[DOWNLOADER]: DONE with DOWNLOADING\n";
			make_final_file(cd);
			pthread_exit(NULL);
			break;
		} catch ( SocketException& e ) {
			cout << "[DOWNLOADER] waiting for uploader to join"<< "\n";
			sleep(2);
		} 
	}
	make_final_file(cd);
	pthread_exit(NULL);
}

void build_fchunks(vector<char *> &fcs, vector <bool> &is_fc, int n) {
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
		client_socket.send(to_string(1));
		int nchunks = atoi(buf);
		build_fchunks(cdata.fchunks, cdata.is_fcpresent, nchunks);

        	memset ( buf, 0, MAXRECV + 1 );		
		client_socket.recv(buf);
		client_socket.send(to_string(1));
		cdata.ofname = buf;
		cout<<"File Name: " << cdata.ofname << " Chunk size: "<<nchunks<<endl;

        	memset ( buf, 0, MAXRECV + 1 );		
		client_socket.recv(buf);
		client_socket.send(to_string(1));
		int num_chunks_iget = atoi(buf);
		cdata.nChunksNeeded = nchunks- num_chunks_iget;
		
		for (int i =0; i < num_chunks_iget; i++) {
			memset ( buf, 0, MAXRECV + 1 );		
			client_socket.recv(buf);
			client_socket.send(to_string(1));
			int cid = atoi(buf);
			int rd = client_socket.recv(cdata.fchunks[cid]);
			client_socket.send(to_string(1));
			cout << "Received CHUNK# "<<cid << " "<< rd <<" Bytes"<<endl;
			cdata.is_fcpresent[cid] = true;
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
	pthread_join(up_th, NULL);
	pthread_join(dw_th, NULL);
   	pthread_exit(NULL);
}
