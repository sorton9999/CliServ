/*
 * serverio.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: root
 */

#include <signal.h>
#include <fcntl.h>
#include "server_io.h"

using namespace std;


server_io* server_io::_instance = NULL;
bool server_io::_loop = true;
const int server_io::MINPORT = 2000;
const int server_io::MAXPORT = 65535;
const int server_io::LISTEN_BACKLOG = 10;

server_io::server_io()
: _slen(0), _listenFd(0), _portId(0), _running(false)
{
	// TODO Auto-generated constructor stub
}

server_io::~server_io() {
	// TODO Auto-generated destructor stub
	close(_listenFd);
}

server_io* server_io::Instance()
{
	if (_instance == NULL)
		_instance = new server_io();
	return _instance;
}

bool server_io::SetupSocket(int port)
{
	_portId = port;

	// Check value of port
	if ((_portId > MAXPORT) || (_portId < MINPORT))
	{
		cerr << "Port Value Not Within Limits: " << _portId << endl;
		return false;
	}

	// Create socket
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
	{
		perror("Cannot Open socket");
		return false;
	}

	memset((void*) &_servAdd, 0, sizeof(_servAdd));
	_servAdd.sin_family = AF_INET;
	_servAdd.sin_addr.s_addr = INADDR_ANY;
	_servAdd.sin_port = htons(_portId);

	// Bind socket
	if (bind(_listenFd, (struct sockaddr*)&_servAdd, sizeof(_servAdd)) < 0)
	{
		perror("Cannot Bind");
		return false;
	}

	// Listen for client connections
	if (listen(_listenFd, LISTEN_BACKLOG) < 0)
	{
		perror("Listen Error");
		return false;
	}
	_running = true;
	return true;
}

void server_io::Start()
{
	fd_set readset;
	int result = -1;
	struct timeval tv;

	while (_running)
	{
		FD_ZERO(&readset);
		FD_SET(_listenFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		result = select(_listenFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0)
		{
			struct sockaddr_in clntAdd;
			_slen = sizeof(clntAdd);
			//cout << "Accepting New Clients." << endl;
			int connFd = accept(_listenFd, (struct sockaddr*)&clntAdd, &_slen);
			if (connFd < 0)
			{
				perror("Client Connection");
			}
			else
			{
				cout << "Connection to FD: " + connFd << endl;
			}
			pthread_t threadId = 0;
			if (_running && pthread_create(&threadId, NULL, server_io::ClientReadTask, &connFd) == 0)
			{
				client_store.push_back(threadId);
			}
		}
	}
	CleanUp();
}

void server_io::Stop()
{
	cout << "Stopping Server Listening on FD: " << _listenFd << endl;
	_running = false;
	_loop = false;
}

void server_io::CleanUp()
{
	cout << "Cleaning up Server resources." << endl;
	for (vector<pthread_t>::iterator iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		char* msg = NULL;
		if (pthread_join(*iter, (void**)&msg))
		{
			perror("pthread_join");
		}
		cout << msg << endl;
		delete msg;
		cout << "Killing Thread: " << *iter << endl;
		if (pthread_kill(*iter, 0) == 0)
		{
			cout << "\n\tKilled." << endl;
		}
	}
}

bool server_io::SignalSetup(void (*handler)(int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigset_t mask;

	sigemptyset(&mask);
	sa.sa_mask = mask;
	sa.sa_handler = handler;
	if (sigaction(SIGINT, &sa, NULL) < 0)
	{
		perror("sigaction");
		return false;
	}
	if (sigaction(SIGTERM, &sa, NULL) < 0)
	{
		perror("sigaction");
		return false;
	}
	return true;
}

void* server_io::ClientReadTask(void* arg)
{
	int clientFd = *((int*)arg);
	pthread_t myId = pthread_self();
	cout << "Start Thread No: " << myId << endl;
	char test[300];
	memset((void*)&test, 0, sizeof(test));
	bool lloop = true;
	fd_set readset;
	struct timeval tv;
	while (_loop && lloop)
	{
		FD_ZERO(&readset);
		FD_SET(clientFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int result = select(clientFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0)
		{
			read(clientFd, test, 300);

			cout << clientFd << ": " << test << endl;

			if (string(test).compare("exit") == 0)
				lloop = false;

			memset((void*)&test, 0, sizeof(test));
		}
	}
	close(clientFd);
	char *msg = new char[100];
	sprintf(msg, "Closing Thread No: %lu, Client FD: %d", myId, clientFd);
	return msg;
}


