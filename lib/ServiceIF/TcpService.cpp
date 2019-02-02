/*
 * TcpService.cpp
 *
 *  Created on: Jun 9, 2018
 *      Author: sorton
 */

#include "TcpService.h"

using namespace std;

namespace service_if {

	const int TcpService::MAXPORT = 65536;
	const int TcpService::MINPORT = 2000;
	const int TcpService::LISTEN_BACKLOG = 10;
	const int TcpService::EXIT_SUCCES = 0;
	const int TcpService::EXIT_FAIL = -1;

	TcpService::TcpService(ConnectionTypeEnum type, const string& hostname, const int port)
	:
			ServiceIF(),
			_hostname(hostname), _portId(port), _type(type), _listenFd(0), _looprun(false)
	{
	}

	TcpService::~TcpService()
	{
	}

	int TcpService::SendMsg(string msg)
	{
		return send (_listenFd, (void*)msg.c_str(), msg.length(), 0);
	}

	string TcpService::GetMsg(void)
	{
		string retStr("");

		return retStr;
	}

	bool TcpService::SetupSocket()
	{
		struct hostent *hostinfo = NULL;

		// Check type.  Only the SERVER should call this.
		if (_type != CONN_SERVER)
		{
			std::cerr << "Connection Type not SERVER!  Setup not run." << std::endl;
			return false;
		}
		// Check value of port
		if ((_portId > MAXPORT) || (_portId < MINPORT))
		{
			std::cerr << "Port Value Not Within Limits: " << _portId << std::endl;
			return false;
		}

		// Create socket
		_listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (_listenFd < 0)
		{
			perror("Cannot Open socket");
			return false;
		}


		// set up the server address struct
		memset((void*) &_servAdd, 0, sizeof(_servAdd));
		_servAdd.sin_family = AF_INET;
		_servAdd.sin_port = htons (_portId);
		//_servAdd.sin_addr.s_addr = INADDR_ANY;

		const char* hostname = NULL;
		if (_hostname.length() == 0)
		{
			hostname = "localhost";
		}
		else
		{
			hostname = _hostname.c_str();
		}
		hostinfo = gethostbyname (hostname);
		if (hostinfo == NULL)
		{
		  fprintf (stderr, "Unknown host %s.\n", hostname);
		  return false;
		}
		_servAdd.sin_addr = *(struct in_addr *) hostinfo->h_addr;

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

		return true;

	}

	int  TcpService::AcceptConnection(struct sockaddr*& clientAddress)
	{
		socklen_t slen = 0;
		return (accept(_listenFd, clientAddress, &slen));
	}

	bool TcpService::SetupClient()
	{

		// Check type.  Only the CLIENT should call this.
		if (_type != CONN_CLIENT)
		{
			return false;
		}
		// Check value of port
		if ((_portId > MAXPORT) || (_portId < MINPORT))
		{
			std::cerr << "Port Value Not Within Limits: " << _portId << std::endl;
			return false;
		}
		// create client socket
		_listenFd = socket(AF_INET, SOCK_STREAM, 0);

		if(_listenFd < 0)
		{
			cerr << "Cannot open socket" << endl;
			return -1;
		}

		_server = gethostbyname(_hostname.c_str());

		if(_server == NULL)
		{
			cerr << "Host does not exist" << endl;
			return false;
		}


		return true;
	}

	bool TcpService::ClientConnect()
	{
		// Check type.  Only the CLIENT should call this.
		if (_type != CONN_CLIENT)
		{
			return false;
		}
		bzero((char *) &_servAdd, sizeof(_servAdd));
		_servAdd.sin_family = AF_INET;

		bcopy((char *) _server -> h_addr, (char *) &_servAdd.sin_addr.s_addr, _server -> h_length);

		_servAdd.sin_port = htons(_portId);

		int connVal = connect(_listenFd,(struct sockaddr *) &_servAdd, sizeof(_servAdd));

		if (connVal < 0)
		{
			cerr << "Cannot connect!  No Server?" << endl;
			return false;
		}

		return true;
	}


	int TcpService::ServiceLoop(struct timeval* looptime)
	{
		FD_ZERO(&_readset);
		FD_SET(_listenFd, &_readset);
		return (select(_listenFd + 1, &_readset, NULL, NULL, looptime));
	}

} /* namespace service_if */
