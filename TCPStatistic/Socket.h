#pragma once

#include "resource.h"

#define BUFLEN 256  //Max length of buffer
class Socket
{
private:
	Socket(int Port, char* Addr,IPPROTO Soctype);
	int Portnum;
	char addr[17];
	char buf[BUFLEN];
	SOCKET s;
	struct sockaddr_in server;
	
	void SendData(int Port, char* DestAddr);
	void ReceiveData();
};
