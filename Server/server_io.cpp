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
		_service = new service_if::TcpService(service_if::TcpService::CONN_SERVER, "", _portId);
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
				std::cout << "Connection to FD: " + connFd << std::endl;
			}
			pthread_t threadId = 0;
			if ((_parser != NULL) && _parser->StartThread(&connFd))
			{
				client_store.push_back(_parser->ThreadId());
			}
			//if (_running && pthread_create(&threadId, NULL, server_io::ClientReadTask, &connFd) == 0)
			//{
			//	client_store.push_back(threadId);
			//}

			sleep(1);
			// Send a ready out to the client
			//if (send (connFd, "READY", 5, 0) < 0)
			if (_service->SendMsg("READY") < 0)
			{
				perror("send to client");
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
			string msg = _udpService->GetMsg();
			cout << "Msg: " << msg << endl;
		}
	}
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
	for (std::vector<pthread_t>::iterator iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		char* msg = NULL;
        bool killThread = false;
		//if (pthread_join(*iter, (void**)&msg))
        if (_parser->WaitForThread((void**)&msg))
		{
			perror("pthread_join");
            killThread = true;
		}
		std::cout << msg << std::endl;
		delete msg;

        if (killThread)
        {
		    std::cout << "Killing Thread: " << *iter << std::endl;
		    //if (pthread_kill(*iter, 0) == 0)
		    if (_parser->KillThread())
		    {
			    std::cout << "\n\tKilled." << std::endl;
		    }
        }
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

/*
 * ClientReadTask -- The client thread processing method.  In the loop two flags are
 *                   checked.  A local flag that gets set to FALSE when the client
 *                   send an 'exit' command and a class level flag that is set to
 *                   FALSE when the server is stopping.  A 'select' is set up to
 *                   detect when something is coming in the the client FD.  When
 *                   something comes in, a 'read' is done to get what comes in.  The
 *                   string is just echoed out to STDOUT.
 * INPUTS: void* -- A pointer to input args which needs to be cast to its real type. In
 *                  this case, the client connection FD.
 * OUPUTS: VOID
 * RETURN: void* -- Any return needed to be sent to the caller.  In this case, a message
 *                  string.
 */
void* server_io::ClientReadTask(void* arg)
{
	int clientFd = *((int*)arg);
	pthread_t myId = pthread_self();
	std::cout << "Start Thread No: " << myId << std::endl;
	char test[300];
    size_t bufLen = sizeof(test);
	memset((void*)&test, 0, bufLen);
	bool lloop = true;
	fd_set readset;
	struct timeval tv;
	FD_ZERO(&readset);
	while (_loop && lloop)
	{
		FD_SET(clientFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int result = select(clientFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0 && _loop && lloop)
		{
			size_t rNum = read(clientFd, test, bufLen);

			//std::cout << clientFd << ": [" << rNum << "]: " << test << std::endl;
			printf("%d : [%d]: %s\n", clientFd, rNum, test);

			if (std::string(test).compare(0, 4, "exit") == 0)
            {
                std::cout << ">>>> Received EXIT From Client: " << clientFd << std::endl;
				lloop = false;
            }
			memset((void*)&test, 0, bufLen);
		}
	}
	close(clientFd);
	char *msg = new char[100];
	sprintf(msg, "Closing Thread No: %lu, Client FD: %d", myId, clientFd);
	return msg;
}

