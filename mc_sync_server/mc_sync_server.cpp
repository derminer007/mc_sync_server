// mc_sync_server.cpp: Definiert den Einstiegspunkt für die Anwendung.
//

#include "mc_sync_server.h"

using namespace std;

#define PORT 8090
const int buff_len = 1024;

int main()
{
	char msg[buff_len];
	memset(msg, 0, buff_len);

	try
	{
		serverConnection server = serverConnection(PORT);
		server.acceptClient();
		cout << "Client-IP: " << server.getClientIP() << endl;
		server.recvBuffer(msg, 5);
		cout << msg << endl;
		server.recvFile("erhalten.txt");
		server.closeClient();
		server.stopConn();
	}
	catch(exception& e)
	{
		cout << "Fehler: " << e.what() << endl;
	}

	return 0;
}
