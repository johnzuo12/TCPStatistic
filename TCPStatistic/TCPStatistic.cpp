// TCPStatistic.cpp : Defines the entry point for the console application.
//

#include "resource.h"
#include "TCPStatistic.h"
#include "Socket.h"
#include <stdio.h>
#include <iostream>

void NetworkCom()
{
	Socket UDP = Socket(2200, "192.168.0.80", IPPROTO_UDP);
	UDP.SendData(2200, "192.168.0.59");
}

int main() {


	//test RTT
	UINT ip = inet_addr("192.168.0.59");
	ULONG hopCount = 0;
	ULONG RTT = 0;
	if (GetRTTAndHopCount(ip, &hopCount, 30, &RTT) == TRUE) {
		printf("Hops: %ld\n", hopCount);
		printf("RTT: %ld\n", RTT);
	}
	else {
		printf("Error: %ld\n", GetLastError());
	}
	getchar();
}


