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
#include <sys/types.h>
#include <sys/socket.h>

#include "ClientParser.h"

using namespace std;

ClientParser::ClientMap ClientParser::client_store;

ClientParser::ClientParser()
{
}

ClientParser::~ClientParser()
{
}

void ClientParser::CleanUp(void)
{
	for (map<pthread_t, ClientData>::iterator iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		char* msg = NULL;
        bool killThread = false;
        if (WaitForThread((void**)&msg))
		{
			perror("pthread_join");
            killThread = true;
		}
		cout << msg << endl;
		delete msg;

        if (killThread)
        {
		    cout << "Killing Thread: " << iter->second.ClientHandle << endl;
		    if (KillThread())
		    {
			    std::cout << "\n\tKilled." << std::endl;
		    }
        }
	}

}

ClientParser::ClientIter ClientParser::FindClientByName(std::string name)
{
	for (ClientIter iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		if (iter->second.ClientName == name)
		{
			return iter;
		}
	}
	return client_store.end();
}

ClientParser::ClientIter ClientParser::FindClientByHandle(pthread_t handle)
{
	for (ClientIter iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		if (iter->second.ClientHandle == handle)
		{
			return iter;
		}
	}
	return client_store.end();
}

void* ClientParser::ThreadReactor(void)
{
	int clientFd = *((int*)_args);
	pthread_t myId = pthread_self();
	cout << "Start Thread No: " << myId << " on FD: " << clientFd << endl;

	// Let's store some of our data
	ClientData data;
	data.ClientHandle = myId;
	data.ClientFd = clientFd;
	Mutex.lock();
	client_store.insert(std::pair<pthread_t, ClientData>(myId, data));
	Mutex.unlock();

	char test[300];
    size_t bufLen = sizeof(test);
	memset((void*)&test, 0, bufLen);
	bool loop = true;
	fd_set readset;
	struct timeval tv;
	FD_ZERO(&readset);
	while (loop)
	{
		FD_SET(clientFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int result = select(clientFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0 && loop)
		{
			//size_t rNum = read(clientFd, test, bufLen);
			int rNum = recv (clientFd, (void*)test, bufLen, MSG_DONTWAIT);

			//std::cout << clientFd << ": [" << rNum << "]: " << test << std::endl;
			printf("%d : [%d]: %s\n", clientFd, rNum, test);

			if (string(test).compare(0, 4, "exit") == 0)
            {
                cout << ">>>> Received EXIT From Client: " << clientFd << endl;
				loop = false;
            }
			else
			{
				ClientIter theData = FindClientByHandle(myId);
				if (theData != client_store.end())
				{
					string bufStr(test);
					size_t pos = bufStr.find(":");
					string cmd = bufStr.substr(0, pos);
					string actor = bufStr.substr(pos+1);
					cout << "Received CMD: \'" << cmd << "\' ACTOR: \'" << actor << "\'" << endl;

					if (cmd == actor)
					{
						cout << "No CMD detected." << endl;
					}
					else if (cmd.compare("Name") == 0)
					{
						cout << "Registering " << actor << endl;
						Mutex.lock();
						theData->second.ClientName = actor;
						Mutex.unlock();
					}
					else
					{
						ClientIter tellData = FindClientByName(cmd);
						if (tellData != client_store.end())
						{
							int wLen = 0;
							int tLen = actor.length();
							int cLen = 0;
							while (cLen < tLen)
							{
								int fd = (int)tellData->second.ClientFd;
								printf ("Sending to FD: %d\n", fd);
								wLen = send (fd, (void*)actor.c_str(), tLen - cLen, MSG_DONTWAIT);
								if (wLen < 0)
								{
									perror ("client send failure");
									break;
								}
								else
								{
									cLen += wLen;
								}
							}
							cout << "Sent [" << actor << "] to [" << cmd << "]" << endl;
						}
						else
						{
							cout << "Name not detected: " << cmd << endl;
						}
					}
				}
			}
			memset((void*)&test, 0, bufLen);
		}
	}
	close(clientFd);
	char *msg = new char[100];
	sprintf(msg, "Closing Thread No: %lu, Client FD: %d", myId, clientFd);
	return msg;
}


