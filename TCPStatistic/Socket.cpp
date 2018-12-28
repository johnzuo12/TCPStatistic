
#include "resource.h"
#include "Socket.h"
#include <stdio.h>
#include <thread>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

Socket::Socket(int Port, const char* Addr, IPPROTO Soctype)
{
	WSADATA wsa;
	int Socktype;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");


	portnum = Port;
	soctype = Soctype;

	memcpy_s(addr, strlen(Addr), Addr, strlen(Addr));
	

	//Create a socket
	if (soctype == IPPROTO_TCP) Socktype = SOCK_STREAM;
	else Socktype = SOCK_DGRAM;
	if ((s = socket(AF_INET, Socktype, soctype)) == INVALID_SOCKET)
	{                                                    
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

}

void Socket::SendData(int Port,const char* DestAddr)
{
	using namespace chrono;
	using system_time = steady_clock;

	struct sockaddr_in si_other;
	char data[256]; // Data to be sent to server

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(Port);
	si_other.sin_addr.S_un.S_addr = inet_addr(DestAddr);

	while (1)
	{
		std::this_thread::sleep_for(milliseconds(2000));
		for (int i = 0; i < BUFLEN / 2;i++)
		{
			data[2 * i] = i;
			data[2 * i + 1] = i*2;
		}

		//send the message
		if (sendto(s, (char*)data, BUFLEN, 0, (struct sockaddr *) &si_other, sizeof(si_other)) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Data sent.\n");
		//
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//puts(buf);
	}

	closesocket(s);
	WSACleanup();

	return;
}

void Socket::ReceiveData()
{
	int recv_len,slen; // can be read from packet
	struct sockaddr_in server,si_other; // can be read from packet
	SOCKET ClientSocket; // connecting socket in TCP

	slen = sizeof(si_other);

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(portnum);

	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) // use global namespace by ::
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	if (soctype == IPPROTO_TCP) {

		if (listen(s, SOMAXCONN) == SOCKET_ERROR)
		{
			printf("listen failed with error code : %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		ClientSocket = accept(s, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// No longer need server socket, use ClientSocket to continue connection 
		closesocket(s);
	}
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) > 0) //no error in receiving
		{
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			uint16_t data[BUFLEN / 2];
			printf("Data: ");

			uint8_t x1 = 0;
			uint8_t x2 = 0;

			for (int i = 0;i < BUFLEN / 2;i++)
			{
				x1 = buf[2 * i];
				x2 = buf[2 * i + 1];
				data[i] = x1 * 256 + x2;
				printf("%d ", data[i]);
			}
			printf("\n");
		}

		else  if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) = SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Connection closing...\n"); //recv_len=0;
			return;
		}
		
	}

	if (soctype == IPPROTO_TCP) closesocket(ClientSocket);
	else closesocket(s);
	WSACleanup();

	return;
}
