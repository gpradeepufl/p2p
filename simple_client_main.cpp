#include "ClientSocket.h"
#include "SocketException.h"
#include <iostream>
#include <string>

using namespace std;

int main ( int argc, int argv[] )
{
  try
    {

      ClientSocket client_socket ( "localhost", 30000 );
      string reply;
      try
	{
	  client_socket << "Test message: Pradeep Kumar Goudagunta";
	  client_socket >> reply;
	}
      catch ( SocketException& ) {}

      cout << "We received this response from the server:\n\"" << reply << "\"\n";;

    }
  catch ( SocketException& e )
    {
      cout << "Exception was caught:" << e.description() << "\n";
    }

  return 0;
}
