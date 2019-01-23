#pragma once

#define CLIENT "169.254.170.82"  //Surface Pro
//#define CLIENT "169.254.243.161" //Dell Notebook
#define SERVER "169.254.100.119" //IPT-N-0245
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
void HeadReader();