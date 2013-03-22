// 463_hw_1_3.cpp : Defines the entry point for the console application.
//
/*
	Author: Ke Wang
	UIN: 822002009
*/
#include "stdafx.h"
#include "crawler.h"
#include <iostream>


int _tmain(int argc, _TCHAR* argv[])
{
	if(argc != 4)
		printf("Invalid input, please try again.\n");
	

	string url(argv[1]);
	int Thread_num = atoi(argv[2]);
	int Page_num = atoi(argv[3]);

	cout<<"Starting "<<Thread_num<<" thread(s) from "<<url<<"\n";

	Crawler c(url,Page_num);
	
	c.StartThreads(Thread_num);
	return 0;
}

