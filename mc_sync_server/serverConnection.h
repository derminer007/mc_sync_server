#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <string.h>
#include <iostream>
#include <exception>
#include "myexceptions.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

using namespace std;

/// <summary>
/// print to stderr
/// </summary>
/// <param name="nachricht">string to be printed</param>
void err_print(const char* nachricht);

/// <summary>
/// Cleanup WSA and exit with -1
/// </summary>
void err_close();

class serverConnection
{
private:
	WSADATA wsaData;
	int iResult;

	SOCKET conn_sock;
	SOCKET client_sock;
	string client_ip;
	int s_port;
	bool failed;

	void setFail();

public:
	serverConnection(int port);
	~serverConnection();

	void startConn();
	void stopConn();
	void acceptClient();
	void closeClient();

	bool getFail();
	const char* getClientIP();

	int sendBuffer(const char* buffer, int buffSize, int chunkSize = 4 * 1024);
	int recvBuffer(char* buffer, int buffSize, int chunkSize = 4 * 1024);
	int sendFile(const char* filename, int chunkSize = 64 * 1024);
	int recvFile(const char* filename, int chunkSize = 64 * 1024);
	int64_t getFileSize(const char* filename);
	/*string getFileHash();*/
};

