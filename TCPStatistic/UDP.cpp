//UDP application
#include "resource.h"
#include <stdio.h>
#include <thread>
#include <winsock2.h>
#include <iostream>
#include <sstream>


#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma warning(disable : 4996)

using namespace std;

void UDPServer()
	{
		SOCKET s;
		struct sockaddr_in server, si_other;
		int slen, recv_len;
		char buf[BUFLEN];
		WSADATA wsa;

		slen = sizeof(si_other);

		//Initialise winsock
		printf("\nInitialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			printf("Failed. Error Code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Initialised.\n");

		//Create a socket
		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
		{
			printf("Could not create socket : %d", WSAGetLastError());
		}
		printf("Socket created.\n");

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(UDPPORT);

		//Bind
		if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("Bind failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		puts("Bind done");

		//keep listening for data
		while (1)
		{
			//printf("Waiting for data...");
			fflush(stdout);

			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);

			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			/*stringstream String2DecimalConverter;
		
			union
			{
				int x;
				float y;
			}converter;*/

			//String2DecimalConverter << buf;
			//String2DecimalConverter >> hex >> x;
			/*uint16_t data[BUFLEN/2];
			printf("Data: ");
			uint8_t x1 = 0;
			uint8_t x2 = 0;

			for (int i = 0;i < BUFLEN/2;i++)
			{
				x1 = buf[2 * i];
				x2 = buf[2 * i + 1];
				data[i] = x1*256 + x2;
			printf("%d ", data[i]);
			}*/
			//printf("%s\n",buf,recv_len);
			
		}

		closesocket(s);
		WSACleanup();

		return;
}

void UDPClient() {
	using namespace chrono;
	using system_time = steady_clock;
	int j = 0;

	struct sockaddr_in si_other;
	SOCKET s;
	int slen = sizeof(si_other);
	char buf[BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("socket created\n");

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(UDPPORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//start communication
	while (1)
	{
		/*std::this_thread::sleep_for(milliseconds(20));
		for (int i = 0; i < BUFLEN / 2;i++)
		{
			buf[2 * i] = i + j;
			buf[2 * i + 1] = i + j;
		}
		j++;*/
		//send the message

		if (sendto(s, "UDP test", BUFLEN, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("data sent\n");
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//puts(buf);
	}

	closesocket(s);
	WSACleanup();

	return;
}


