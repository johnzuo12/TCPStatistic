// TCPStatistic.cpp : Defines the entry point for the console application.
//

#include "resource.h"
#include "TCPStatistic.h"
#include <stdio.h>
#include <iostream>



int main() {


	//test RTT
	UINT ip = inet_addr("127.0.0.1");
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


