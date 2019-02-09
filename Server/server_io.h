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

#include <TcpService.h>
#include <UdpService.h>
#include "ClientParser.h"

class server_io {
public:

	virtual ~server_io();
	static server_io* Instance(int port, std::string host);

	bool SetupSocket(void);
	void Start();
	void Stop();
	void CleanUp();

	bool   SetupUdpService();
	void   StartUdpService();
	int    SendMsg(int fd, std::string msg, unsigned int flags);
	int    GetMsg(int fd, char** msg, unsigned long len, unsigned int flags);

	static bool SignalSetup(void (*handler)(int));

protected:

	server_io(int port, std::string host);

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
