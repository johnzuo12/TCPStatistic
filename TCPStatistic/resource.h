#pragma once

#define SERVER "169.254.170.82"  //ip address of TCP server
#define CLIENT "169.254.243.161"
#define BUFLEN 256  //Max length of buffer
#define TCPSERVERPORT 55005  
#define TCPCLIENTPORT 52005
#define UDPPORT 55000   

void UDPServer();
void UDPClient();
void TCPServer();
void TCPClient();
void RTTTest();
void LocalTCPStatistics();
void RunEstatsTest();