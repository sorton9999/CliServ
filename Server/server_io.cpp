/*
 * server_io.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: sorton
 */

#include <signal.h>
#include <iostream>
#include "server_io.h"

using namespace std;


server_io* server_io::_instance = NULL;
bool server_io::_loop = true;
const int server_io::MINPORT = 2000;
const int server_io::MAXPORT = 65535;
const int server_io::LISTEN_BACKLOG = 10;

/*
 * Constructor
 */
server_io::server_io()
: _listenFd(0), _portId(0), _running(false)
{
	// TODO Auto-generated constructor stub
}

/*
 * Destructor
 */
server_io::~server_io() {
	// TODO Auto-generated destructor stub
	close(_listenFd);
}

/*
 * Instance -- This class follows the singleton pattern so this is
 *             the public method to get a pointer to the class.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: server_io -- A pointer to this class
 */
server_io* server_io::Instance()
{
	if (_instance == NULL)
		_instance = new server_io();
	return _instance;
}

/*
 * SetupSocket -- Set up the port connection, open a socket and
 *                listen.
 * INPUTS: int -- The port to connect with
 * OUPUTS: VOID
 * RETURN: bool -- The success or failure of this operation
 */
bool server_io::SetupSocket(int port)
{
	_portId = port;

	// Check value of port
	if ((_portId > MAXPORT) || (_portId < MINPORT))
	{
		cerr << "Port Value Not Within Limits: " << _portId << endl;
		return false;
	}

	// Create socket
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
	{
		perror("Cannot Open socket");
		return false;
	}

	memset((void*) &_servAdd, 0, sizeof(_servAdd));
	_servAdd.sin_family = AF_INET;
	_servAdd.sin_addr.s_addr = INADDR_ANY;
	_servAdd.sin_port = htons(_portId);

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
	_running = true;
	return true;
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
	fd_set readset;
	int result = -1;
	socklen_t slen = 0;
	struct timeval tv;

	while (_running)
	{
		FD_ZERO(&readset);
		FD_SET(_listenFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		result = select(_listenFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0)
		{
			struct sockaddr_in clntAdd;
			slen = sizeof(clntAdd);
			int connFd = accept(_listenFd, (struct sockaddr*)&clntAdd, &slen);
			if (connFd < 0)
			{
				perror("Client Connection");
			}
			else
			{
				cout << "Connection to FD: " + connFd << endl;
			}
			pthread_t threadId = 0;
			if (_running && pthread_create(&threadId, NULL, server_io::ClientReadTask, &connFd) == 0)
			{
				client_store.push_back(threadId);
			}

            sleep(1);
            // Send a ready out to the client
            if (send (connFd, "READY", 5, 0) < 0)
            {
                perror("send to client");
            }
		}
	}
	CleanUp();
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
	cout << "Stopping Server Listening on FD: " << _listenFd << endl;
	// stop the main loop
	_running = false;
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
	cout << "Cleaning up Server resources." << endl;
	for (vector<pthread_t>::iterator iter = client_store.begin();
		 iter != client_store.end();
		 ++iter)
	{
		char* msg = NULL;
        bool killThread = false;
		if (pthread_join(*iter, (void**)&msg))
		{
			perror("pthread_join");
            killThread = true;
		}
		cout << msg << endl;
		delete msg;

        if (killThread)
        {
		    cout << "Killing Thread: " << *iter << endl;
		    if (pthread_kill(*iter, 0) == 0)
		    {
			    cout << "\n\tKilled." << endl;
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
	cout << "Start Thread No: " << myId << endl;
	char test[300];
    size_t bufLen = sizeof(test);
	memset((void*)&test, 0, bufLen);
	bool lloop = true;
	fd_set readset;
	struct timeval tv;
	while (_loop && lloop)
	{
		FD_ZERO(&readset);
		FD_SET(clientFd, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int result = select(clientFd + 1, &readset, NULL, NULL, &tv);
		if (result > 0 && _loop && lloop)
		{
			read(clientFd, test, bufLen);

			cout << clientFd << ": " << test << endl;

			if (string(test).compare(0, 4, "exit") == 0)
            {
                cout << ">>>> Received EXIT From Client: " << clientFd << endl;
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

