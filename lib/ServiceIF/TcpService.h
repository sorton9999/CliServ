/*
 * TcpService.h
 *
 *  Created on: Jun 9, 2018
 *      Author: sorton
 */

#ifndef TCPSERVICE_H_
#define TCPSERVICE_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ServiceIF.h"

namespace service_if {

class TcpService: public service_if::ServiceIF
{
public:
	static const int MAXPORT;
	static const int MINPORT;
	static const int LISTEN_BACKLOG;
	static const int EXIT_SUCCES;
	static const int EXIT_FAIL;

	enum ConnectionTypeEnum { CONN_UNINIT, CONN_SERVER, CONN_CLIENT };

	TcpService(ConnectionTypeEnum type, const std::string& hostname, const int port);
	virtual ~TcpService();
	virtual int SendMsg(std::string msg);
	virtual std::string GetMsg(void);

	bool SetupSocket();
	int  AcceptConnection(struct sockaddr*& clientAddress);
	bool SetupClient();
	bool ClientConnect();
	int  ServiceLoop(struct timeval* looptime);

	std::string HostName(void) { return _hostname; }
	int         Port(void) { return _portId; }
	int         ListenFd(void) { return _listenFd; }

protected:
	TcpService(const TcpService& orig)
	: _portId(orig._portId), _listenFd(-1), _hostname(orig._hostname),
	  _looprun(false), _type(orig._type)
	{
		memcpy(&_servAdd, 0, sizeof(_servAdd));
		_server->h_addr = NULL;
	}

private:
	int                _portId;
	int                _listenFd;
	std::string        _hostname;
	struct sockaddr_in _servAdd;
    struct hostent *   _server;
	bool               _looprun;
	fd_set             _readset;
	ConnectionTypeEnum _type;

};

} /* namespace service_if */
#endif /* TCPSERVICE_H_ */
