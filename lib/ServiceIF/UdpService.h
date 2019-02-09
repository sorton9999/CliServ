#ifndef UDPSERVICE_H_
#define UDPSERVICE_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ServiceIF.h"

namespace service_if
{

class UdpService : public service_if::ServiceIF
{
public:
	static const int MAXPORT;
	static const int MINPORT;
	static const int LISTEN_BACKLOG;

	enum ConnectionTypeEnum { CONN_UNINIT, CONN_SERVER, CONN_CLIENT };

	UdpService(ConnectionTypeEnum type, const std::string& hostname, const int port);
	virtual ~UdpService();

	virtual int   SendMsg(std::string message);
	virtual int   SendMsg(int fd, std::string msg, unsigned int flags);
	virtual int GetMsg(char** msgRef, unsigned long bufLen);
	virtual int GetMsg(int fd, char** msgRef, unsigned long bufLen, unsigned int flags);

	bool SetupSocket();
	int ServiceLoop(struct timeval* looptime);

	std::string HostName(void) { return _hostname; }
	int         Port(void) { return _portId; }
	int         ListenFd(void) { return _listenFd; }

protected:
	UdpService(const UdpService& orig)
	{
		_portId = orig._portId;
		_listenFd = -1;
		_hostname = orig._hostname;
		_looprun = false;
		_connType = orig._connType;
		_msgSizeSent = 0;
		_numBytesRecv = 0;
		memcpy(&_servAdd, 0, sizeof(_servAdd));
		memcpy(&_server, &orig._server, sizeof(_server));
	}

private:
	int                _portId;
	int                _listenFd;
	std::string        _hostname;
	struct sockaddr_in _servAdd;
	struct hostent *   _server;
	struct addrinfo *  _addrinfo;
	bool               _looprun;
	fd_set             _readset;
	ConnectionTypeEnum _connType;
	int                _msgSizeSent;
	int                _numBytesRecv;
};

} /* namespace service_if */
#endif /* UDPSERVICE_H_ */
