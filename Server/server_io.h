/*
 * serverio.h
 *
 *  Created on: Aug 23, 2014
 *      Author: sorton
 */

#ifndef SERVERIO_H_
#define SERVERIO_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>


class server_io {
public:
	static const int MAXPORT;
	static const int MINPORT;
	static const int LISTEN_BACKLOG;

	virtual ~server_io();
	static server_io* Instance();

	bool SetupSocket(int port);
	void Start();
	void Stop();
	void CleanUp();

	static void* ClientReadTask(void* arg);
	static bool SignalSetup(void (*handler)(int));

protected:
	std::vector<pthread_t> client_store;

	server_io();

	static void ClientHandler(int signum);

private:
	static server_io* _instance;
	static bool _loop;

	int _portId;
	int _listenFd;
	struct sockaddr_in _servAdd;
	bool _running;
};

#endif /* SERVERIO_H_ */
