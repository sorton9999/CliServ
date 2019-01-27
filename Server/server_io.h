/*
 * serverio.h
 *
 *  Created on: Aug 23, 2014
 *      Author: sorton
 */

#ifndef SERVERIO_H_
#define SERVERIO_H_

#include <pthread.h>
#include <vector>

#include "TcpService.h"
#include "UdpService.h"
#include "ClientParser.h"

class server_io {
public:

	virtual ~server_io();
	static server_io* Instance(int port, std::string host);

	bool SetupSocket(void);
	void Start();
	void Stop();
	void CleanUp();

	bool SetupUdpService();
	void StartUdpService();

	static void* ClientReadTask(void* arg);
	static bool SignalSetup(void (*handler)(int));

protected:
	std::vector<pthread_t> client_store;

	server_io(int port, std::string host);

	//bool SetupUdpSocket();

	static void ClientHandler(int signum);

private:

	// TCP vars
	int                _portId;
	std::string        _hostname;
	int                _listenFd;
	//struct sockaddr_in _servAdd;
	bool               _running;
	service_if::TcpService* _service;

	// UDP vars
	int _udpPort;
	int _udpFd;
	bool _udpRunning;
	service_if::UdpService* _udpService;

	ClientParser* _parser;

	static server_io*    _instance;
	static bool          _loop;

};

#endif /* SERVERIO_H_ */
