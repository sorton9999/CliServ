#include <signal.h>
#include <iostream>
#include "server_io.h"
#include "TcpService.h"

using namespace std;
using namespace service_if;

// Prototypes
static void ServerStop();
static bool ParseArgs(int argc, char*argv[]);
static void PrintUsage();

// Default port
const int DFLT_PORT = 2111;
int port = 0;

// Host name
const std::string DFLT_HOST = "localhost";
std::string hostname = "";

// A pointer to the server object
static server_io* svr = NULL;

// TCP or UDP
bool startTCP = false;
bool startUDP = false;


/*
 * my_sig_handler -- The signal handler set up for stopping this
 *                   application.
 * INPUTS: int -- The signal that set off this handler
 * OUTPUTS: VOID
 * RETURN: VOID
 */
static void my_sig_handler(int sig_num)
{
	if (sig_num == SIGINT || sig_num == SIGTERM)
		ServerStop();
}

/*
 * main -- The main program entrance.  A port is read from the command arguments
 *         if one is sent in.  If not, a default value is set and used in server
 *         setup.
 * INPUTS: int -- the number of arguments sent to the application from the
 *                command line
 *         char** -- The arguments
 * OUTPUTS: VOID
 * RETURN: int -- The state of the program when exiting.  A program error or failure
 *                will return EXIT_FAILURE.  A normal program exit will return
 *                EXIT_SUCCESS.
 */
int main (int argc, char**argv)
{
	int retVal = TcpService::EXIT_SUCCES;
	if (!server_io::SignalSetup(my_sig_handler))
	{
		cerr << "Signals not set up correctly\n";
		return TcpService::EXIT_FAIL;
	}

	if (!ParseArgs(argc, argv))
	{
		return 0;
	}

	printf("Starting Server on Port: %d\n", port);

	svr = server_io::Instance(port, hostname);
	if (svr != NULL)
	{
		if (startTCP)
		{
			if (svr->SetupSocket())
			{
				cout << "Starting TCP Service." << endl;
				svr->Start();
			}
			else
			{
				cout << "TCP Service Start Failed!" << endl;
				retVal = TcpService::EXIT_FAIL;
			}
		}
		if (startUDP)
		{
			if (svr->SetupUdpService())
			{
				cout << "Starting UDP Service." << endl;
				svr->StartUdpService();
			}
			else
			{
				cout << "UDP Service Start Failed!" << endl;
				retVal = TcpService::EXIT_FAIL;
			}
		}
	}
	return retVal;
}

static bool ParseArgs(int argc, char* argv[])
{
	bool argsParsed = false;
	for (int i = 1; i < argc; ++i)
	{
		argsParsed = true;
		if (strcmp(argv[i], "--help") == 0)
		{
			PrintUsage();
			return false;
		}
		if (strcmp(argv[i], "-H") == 0)
		{
			hostname = argv[i + 1];
		}
		if (strcmp(argv[i], "-P") == 0)
		{
			port = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "-T") == 0)
		{
			startTCP = true;
		}
		if (strcmp(argv[i], "-U") == 0)
		{
			startUDP = true;
		}
	}
	if (!argsParsed)
	{
		port = DFLT_PORT;
		hostname = DFLT_HOST;
		startTCP = true;
	}
	return true;
}

static void PrintUsage()
{
	cout << "Server: "
			<< "[-P port] "
			<< "[-H hostname] "
			    << "[-T(cp)] [-U(dp)]"
			               << endl;
}

/*
 * ServerStop -- The server stop method called from the signal handler in
 *               response to terminate signals.
 * INPUTS: VOID
 * OUTPUTS: VOID
 * RETURN: VOID
 */
static void ServerStop()
{
	cout << "Stopping Server" << endl;
	if (svr != NULL)
		svr->Stop();
}



