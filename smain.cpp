#include <iostream>
#include <stdio.h>
#include "ServerSocket.h"
#include "SocketException.h"
#include <string.h>

using namespace std;

int main ( int argc, int argv[] )
{
  FILE *infp = fopen("in.txt", "r" );
  std::cout << "running....\n";
 char buf[MAXRECV];
memset ( buf, 0, MAXRECV + 1 );
  try
    {
      // Create the socket
      ServerSocket server ( 30000 );

     // while ( true )
	{

	  ServerSocket new_sock;
	  server.accept ( new_sock );
  	  std::cout << "Connected to Client\n";

	  try
	    {
	      while ( true )
		{
                size_t read = fread(buf, 1, MAXRECV, infp);
                if (read < MAXRECV) {
			buf[read] = '\0';
		  	std::string data (buf);
		 	new_sock << data;
                        break;
                }
		std::string data (buf);
		  //new_sock >> data;
  	  	 //std::cout << "Data SENT to Client "<< data <<"\n";
		 //data += "SERVER"; 
		 new_sock << data;
		 memset (buf, 0, MAXRECV + 1 );
		  //std::cout << "Recvd data: "<< data << endl;
		}
	    }
	  catch ( SocketException& ) {}
  sleep(10);
  std::string data ("end");
  new_sock << data;
	}
    }
  catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
    }

  return 0;
}
