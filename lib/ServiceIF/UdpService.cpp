/*
 * UdpService.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: sorton
 */

#include <iostream>

#include "UdpService.h"


namespace service_if
{
	const int UdpService::MAXPORT = 65535;
	const int UdpService::MINPORT = 2000;
	const int UdpService::LISTEN_BACKLOG = 10;

	UdpService::UdpService(ConnectionTypeEnum type, const std::string& hostname, const int port)
		:
		ServiceIF(),
		_hostname(hostname),
		_portId(port),
		_listenFd(0),
		_looprun(false),
		_connType(type),
		_msgSizeSent(0),
		_numBytesRecv(0)
	{

	}

	UdpService::~UdpService()
	{
		freeaddrinfo(_addrinfo);
	}

	bool UdpService::SetupSocket()
	{
		bool retVal = true;

	    char decimal_port[16];
	    snprintf(decimal_port, sizeof(decimal_port), "%d", _portId);
	    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
	    struct addrinfo hints;
	    memset(&hints, 0, sizeof(hints));
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_DGRAM;
	    hints.ai_protocol = IPPROTO_UDP;
	    int r(getaddrinfo(_hostname.c_str(), decimal_port, &hints, &_addrinfo));
	    if(r != 0 || _addrinfo == NULL)
	    {
	    	retVal = false;
	    }

	    // Create the socket file descriptor
	    if (retVal && ((_listenFd = socket(_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP)) < 0) )
	    {
	        perror("socket creation failed");
	        retVal = false;
	    }

	    if (_connType == CONN_SERVER)
	    {
			if (retVal && _listenFd > 0)
			{
				if ( bind(_listenFd, _addrinfo->ai_addr, _addrinfo->ai_addrlen) < 0)
				{
					perror("bind failed");
					close (_listenFd);
					retVal = false;
				}

			}
	    }

		return retVal;
	}

	int UdpService::GetMsg(char** msgRef, unsigned long bufLen)
	{
		if (_connType != CONN_SERVER)
		{
			return -1;
		}
		return GetMsg(_listenFd, msgRef, bufLen, 0);
	}

	int UdpService::GetMsg(int fd, char** msgRef, unsigned long bufLen, unsigned int flags)
	{
		int rNum = -1;
		if ((rNum = recv (fd, (void*)*msgRef, bufLen, flags)) < 0)
		{
			perror ("client recv error");
		}
		if (rNum >= 0)
		{
			(*msgRef)[rNum] = '\0';
		}
		_numBytesRecv = rNum;
		return rNum;
	}

	int UdpService::SendMsg(std::string message)
	{
		int retVal = -1;
		if (_connType != CONN_CLIENT)
		{
			_msgSizeSent = 0;
			return 0;
		}
		_msgSizeSent = 0;
		if ((retVal = sendto(_listenFd, message.c_str(), message.length(), 0, _addrinfo->ai_addr, _addrinfo->ai_addrlen)) < 0)
		{
			perror ("sendto client error");
		}
		_msgSizeSent = retVal;
		return retVal;
	}

	int UdpService::SendMsg(int fd, std::string msg, unsigned int flags)
	{
		int retVal = -1;

		return retVal;
	}

	int UdpService::ServiceLoop(struct timeval* looptime)
	{
		FD_ZERO(&_readset);
		FD_SET(_listenFd, &_readset);
		return (select(_listenFd + 1, &_readset, &_readset, &_readset, looptime));
	}

} /* service_if */


