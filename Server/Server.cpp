#include <signal.h>
#include <iostream>
#include "server_io.h"

using namespace std;

static void ServerStop();

const int DFLT_PORT = 2111;

static server_io* svr = NULL;


static void my_sig_handler(int sig_num)
{
	if (sig_num == SIGINT || sig_num == SIGTERM)
		ServerStop();
}

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
	}
	return EXIT_SUCCESS;
}

static void ServerStop()
{
	cout << "Stopping Server" << endl;
	if (svr != NULL)
		svr->Stop();
}



