// TCPStatistic.cpp : Defines the entry point for the console application.
//

#include "resource.h"
#include <stdio.h>
#include <thread>
#include <iostream>


int main() {
	std::thread t_com(TCPServer);
	//std::thread t_test(RunEstatsTest);
	t_com.join();
	//t_test.join();
	getchar();
	}


