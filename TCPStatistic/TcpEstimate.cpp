#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Iphlpapi.lib and Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable : 4996)

void RTTTest()
{
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
}