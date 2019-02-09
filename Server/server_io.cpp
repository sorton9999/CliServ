/*
 * server_io.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: sorton
 */

//#include <cstdio>
#include <signal.h>
#include <iostream>
#include "server_io.h"
//#include "TcpService.h"
//#include "UdpService.h"

using namespace std;
using namespace service_if;


server_io* server_io::_instance = NULL;
bool server_io::_loop = true;

/*
 * Constructor
 */
server_io::server_io(int port, std::string host)
{
	_listenFd = 0;
	_portId = port;
	_running = false;
	_service = NULL;
	_hostname = host;
	_udpPort = 0;
	_udpService = NULL;
	_udpRunning = false;
	_udpFd = 0;
	_parser = new ClientParser();
}

/*
 * Destructor
 */
server_io::~server_io()
{
	close (_listenFd);
	if (_service != NULL)
		delete _service;
	_service = NULL;
	close (_udpFd);
	if (_udpService != NULL)
		delete _udpService;
	_udpService = NULL;
}

/*
 * Instance -- This class follows the singleton pattern so this is
 *             the public method to get a pointer to the class.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: server_io -- A pointer to this class
 */
server_io* server_io::Instance(int port, std::string host)
{
	if (_instance == NULL)
		_instance = new server_io(port, host);
	return _instance;
}

/*
 * SetupSocket -- Set up the port connection, open a socket and
 *                listen.
 * INPUTS: VOID
 * OUPUTS: VOID
 * RETURN: bool -- The success or failure of this operation
 */
bool server_io::SetupSocket(void)
{
	if (_service == NULL)
	{
		_service = new service_if::TcpService(service_if::TcpService::CONN_SERVER, _hostname, _portId);
	}
	_running = _service->SetupSocket();
	return _running;
}

/*
 * SetupUdpService -- Set up the service-side UDP socket connection.
 *
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: bool -- The success or failure of this operation
 */
bool server_io::SetupUdpService()
{
	if (_udpService == NULL)
	{
		_udpPort = _portId;
		_udpService = new UdpService(UdpService::CONN_SERVER, _hostname, _udpPort);
	}
	_udpRunning = _udpService->SetupSocket();
	return _udpRunning;
}

/*
 * Start -- Starts the applications main processing loop.  The loop runs until
 *          the flag '_running' is set to FALSE in a signal handler.  A select
 *          is set up to detect incoming activity on the '_listenFd' FD.  Once
 *          something comes in, an 'accept' is performed.  Once the connection
 *          to the client is made, a thread is spawned for the client and the
 *          thread ID is put into a class storage data structure.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: VOID
 */
void server_io::Start()
{
	struct sockaddr* clntAdd = new sockaddr();
	int result = -1;
	struct timeval tv;

	while (_running)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		result = _service->ServiceLoop(&tv);
		if (result > 0)
		{
			int connFd = _service->AcceptConnection(clntAdd);
			if (connFd < 0)
			{
				perror("Client Connection");
			}
			else
			{
				printf("%s%d\n", "Connection to FD:", connFd);
			}
			// We got a client connection.  Start a service thread for it.
			if ((_parser != NULL) && _parser->StartThread(&connFd))
			{
				cout << "Started Thread For Client: " << connFd << endl;
			}

			sleep(1);
			// Send a ready out to the client
			if (_service->SendMsg(connFd, "READY", 0) < 0)
			{
				printf("Bad Client Send To FD:%d", connFd);
			}
		}
	}
	delete clntAdd;
	clntAdd = NULL;
	CleanUp();
}

void server_io::StartUdpService()
{
	int result = -1;
	struct timeval tv;

	while (_udpRunning)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		result = _udpService->ServiceLoop(&tv);
		if (result > 0)
		{
			char* msg = new char(256);
			int num = _udpService->GetMsg(&msg, 256);
			cout << "Msg: " << msg << "; Size: " << num << endl;

			_udpService->SendMsg(msg);

			delete[] msg;
			msg = NULL;
		}
	}
}

int server_io::SendMsg(int fd, std::string msg, unsigned int flags)
{
	return _service->SendMsg(fd, msg, flags);
}

int server_io::GetMsg(int fd, char** msg, unsigned long len, unsigned int flags)
{
	return _service->GetMsg(fd, msg, len, flags);
}

/*
 * Stop -- The stop method that sets some class flags to exit out of the main
 *         loop and the client loops.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: VOID
 */
void server_io::Stop()
{
	std::cout << "Stopping Server Listening on FD: " << _listenFd << std::endl;
	// stop the main loop
	_running = false;
	_udpRunning = false;
	// stop all client loops
	_loop = false;
}

/*
 * CleanUp -- Cleanup some connections and client threads before processing
 *            stops.  A 'join' is called for each client thread so it will
 *            return gracefully and stop.  A thread kill is called in case
 *            the thread doesn't stop gracefully.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: VOID
 */
void server_io::CleanUp()
{
	std::cout << "Cleaning up Server resources." << std::endl;
	if (_parser != NULL)
	{
		_parser->CleanUp();
	}
}

/*
 * SignalSetup -- A thread safe signal setup method used to register
 *                signals for that the application will honor when
 *                sent.  The signals SIGTERM and SIGINT are registered
 *                to help the application stop (such as hitting <CTRL-C>
 *                in the terminal window.
 * INPUTS: *handler -- A function pointer to the signal handler
 * OUTPUTS: VOID
 * RETURN: bool -- Whether or not this operation was successful
 */
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
