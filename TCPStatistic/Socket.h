#pragma once

#include "resource.h"

#define BUFLEN 256  //Max length of buffer
class Socket
{
private:
	int portnum;
	char addr[17];
	char buf[BUFLEN];
	SOCKET s;
	IPPROTO soctype;
public:	
	Socket(int Port, const char* Addr, IPPROTO Soctype);
	void SendData(int Port, const char* DestAddr);
	void ReceiveData();
};
