/*
 * ClientParser.cpp
 *
 *  Created on: Jan 26, 2019
 *      Author: sorton
 */

#include <string>
#include <iostream>
#include <signal.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ClientParser.h"


ClientParser::ClientParser()
{
	// TODO Auto-generated constructor stub
	_loop = false;
}

ClientParser::~ClientParser()
{
	// TODO Auto-generated destructor stub
}

void* ClientParser::ThreadReactor(void)
{
	int clientFd = *((int*)_args);
	pthread_t myId = pthread_self();
	std::cout << "Start Thread No: " << myId << std::endl;
	char test[300];
    size_t bufLen = sizeof(test);
	memset((void*)&test, 0, bufLen);
	bool loop = true;
	fd_set readset;
	struct timeval tv;
	FD_ZERO(&readset);
	while (_loop)
	{
		FD_SET(clientFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int result = select(clientFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0 && loop)
		{
			size_t rNum = read(clientFd, test, bufLen);

			//std::cout << clientFd << ": [" << rNum << "]: " << test << std::endl;
			printf("%d : [%d]: %s\n", clientFd, rNum, test);

			if (std::string(test).compare(0, 4, "exit") == 0)
            {
                std::cout << ">>>> Received EXIT From Client: " << clientFd << std::endl;
				loop = false;
            }
			memset((void*)&test, 0, bufLen);
		}
	}
	close(clientFd);
	char *msg = new char[100];
	sprintf(msg, "Closing Thread No: %lu, Client FD: %d", myId, clientFd);
	return msg;
}


