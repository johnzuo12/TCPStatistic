#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include "resource.h"
#include <stdio.h>
#include <thread>
#include <string>
#include <iostream>
#include <sstream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma warning(disable : 4996)

void TCPServer()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct sockaddr_in server,client;
	char recvbuf[BUFLEN];
	int recvbuflen = BUFLEN;
	std::string recvinfo;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("listen num %d\n", ListenSocket);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(TCPSERVERPORT);

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, (struct sockaddr *)&server, sizeof(server));
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	
	int client_size = sizeof(client);

	// Accept a client socket
	ClientSocket = accept(ListenSocket, (struct sockaddr *)&client, &client_size);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	else printf("ClientSocket num %d\n", (int)ClientSocket);

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);
			recvinfo.assign(recvbuf,iResult);
			//printf("received info: %s\n",recvinfo.c_str());


			//// Echo the buffer back to the sender
			//iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			//if (iSendResult == SOCKET_ERROR) {
			//	printf("send failed with error: %d\n", WSAGetLastError());
			//	closesocket(ClientSocket);
			//	WSACleanup();
			//	return;
			//}
			//printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return;
}

void TCPClient()
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct  sockaddr_in server,client;
	char recvbuf[BUFLEN];
	std::string info,sendinfo;
	int iResult;
	int recvbuflen = BUFLEN;
	using namespace std::chrono;
	using system_time = steady_clock;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(SERVER);
	server.sin_port = htons(TCPSERVERPORT);

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	//turn off Nagel algorithm
	int i = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(i));

	//specify a port num for client
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(CLIENT);
	client.sin_port = htons(TCPCLIENTPORT);	
	iResult = bind(ConnectSocket, (struct sockaddr *)&client, sizeof(client));
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, (struct sockaddr *)&server, sizeof(server));
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}
	printf("connected\n");

	info = "just for test #";
	int seq = 0;
	while (1)
	{
		std::this_thread::sleep_for(milliseconds(20));
		sendinfo = info + std::to_string(seq);
		iResult = send(ConnectSocket, sendinfo.c_str(), sendinfo.length(), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return;
		}
		printf("Bytes Sent: %ld\n", iResult);
		seq++;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return;
}