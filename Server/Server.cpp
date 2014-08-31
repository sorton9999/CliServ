#include <signal.h>
#include <iostream>
#include "server_io.h"

using namespace std;

// Prototypes
static void ServerStop();

// Default port
const int DFLT_PORT = 2111;

// A pointer to the server object
static server_io* svr = NULL;


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
int main (int argc, char*argv[])
{
	if (!server_io::SignalSetup(my_sig_handler))
	{
		cerr << "Signals not set up correctly\n";
		return EXIT_FAILURE;
	}

	int port = 0;
	if (argc > 1)
		port = atoi(argv[(argc-1)]);
	else
		port = DFLT_PORT;
	//cout << "Starting Server on Port: " + port << endl;
	printf("Starting Server on Port: %d\n", port);

	svr = server_io::Instance();
	if (svr != NULL)
	{
		if (svr->SetupSocket(port))
			svr->Start();
		else
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
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



