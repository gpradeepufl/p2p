#include "ClientSocket.h"
#include "SocketException.h"
#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;

int main ( int argc, int argv[] )
{
	FILE *op = fopen("out.txt", "w" );
	try
	{
		ClientSocket client_socket ( "localhost", 30000 );
		string reply;
		while (true) {
			try
			{
				cout<<"Getting Resp from server \n";
				client_socket >> reply;
				cout<<"Response: "<<reply<<endl;
				fwrite (reply.c_str(), sizeof(char), reply.size(), op);
			}
			catch ( SocketException& ) {
				cout<<"Response: "<<reply<<endl;
				if (reply == "")
					break;
			}
		}

	}
	catch ( SocketException& e )
	{
		cout << "Exception was caught:" << e.description() << "\n";
	}
	fclose(op);
	return 0;
}
