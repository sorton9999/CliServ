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
			_server(NULL), _type(type), _listenFd(0), _looprun(false)
	{
		_hostname = hostname;
		_portId = port;
	}

	TcpService::~TcpService()
	{
	}

	int TcpService::SendMsg(string msg)
	{
		int retVal = -1;
		if ((retVal =  send (_listenFd, (void*)msg.c_str(), msg.length(), 0)) < 0)
		{
			perror ("send to client");
		}
		return retVal;
	}

	int TcpService::SendMsg(int fd, string msg, unsigned int flags)
	{
		int retVal = -1;
		int wLen = 0;
		int tLen = msg.length();
		char* start = &(msg[0]);
		char* end = &(msg[tLen]);
		while (start < end)
		{
			printf ("Sending to FD: %d; Size: [%d]\n", fd, tLen);
			// typical flag might be MSG_DONTWAIT, otherwise 0 for no flags
			wLen = send (fd, (void*)start, (end - start), flags);
			if (wLen < 0)
			{
				perror ("client send failure");
				break;
			}
			else
			{
				printf ("Sent to FD: %d [%d] bytes\n", fd, wLen);
				start = &(msg[wLen]);
			}
		}
		return retVal;
	}

	int TcpService::GetMsg(char** msgRef, unsigned long bufLen)
	{
		return (GetMsg (_listenFd, msgRef, bufLen, 0));
	}

	int TcpService::GetMsg(int fd, char** msgRef, unsigned long bufLen, unsigned int flags)
	{
		int rNum = -1;
		if ((rNum = recv (fd, (void*)(*msgRef), bufLen, flags)) < 0)
		{
			perror ("recv from client error");
		}
		if (rNum >= 0)
		{
			(*msgRef)[rNum] = '\0';
		}
		return rNum;
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

		const char* hostname = NULL;
		if (_hostname.length() == 0)
		{
			hostname = "localhost";
		}
		else if (_hostname == "localhost")
		{
			hostname = _hostname.c_str();
		}
		else
		{
			_servAdd.sin_addr.s_addr = INADDR_ANY;
			cout << "Connection from Anybody set up" << endl;
		}

		if ((hostname != NULL) && (strcmp(hostname, "localhost") == 0))
		{
			hostinfo = gethostbyname (hostname);
			if (hostinfo == NULL)
			{
			  fprintf (stderr, "Unknown host %s.\n", hostname);
			  return false;
			}
			_servAdd.sin_addr = *(struct in_addr *) hostinfo->h_addr;
			cout << "Connection from localhost only set up" << endl;
		}

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
		else
		{
			printf("\nhost name alias list:\n");
			for (int i = 0; _server->h_aliases[i]; ++i)
			{
				printf("%s\n", _server->h_aliases[i]);
			}

			printf("\nip address list:\n");
			char ip[16];
			struct sockaddr_in sock_addr;
			for (int i = 0; _server->h_addr_list[i]; ++i)
			{
				sock_addr.sin_addr = *((struct in_addr*) _server->h_addr_list[i]);
				inet_ntop(AF_INET, &sock_addr.sin_addr, ip, sizeof(ip));
				printf("%s\n", ip);
			}
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

		if (connect(_listenFd,(struct sockaddr *) &_servAdd, sizeof(_servAdd)) < 0)
		{
			perror("client connect error");
			cerr << "Cannot connect!  No Server at " << _server->h_addr << endl;
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
