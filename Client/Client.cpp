#include <string.h>
#include <cstring>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <time.h>
#include <vector>

#include <TcpService.h>
#include <UdpService.h>

bool SignalSetup(void (*handler)(int));
bool ServerReady(int fd);

// Using TCP or UDP
bool startTCP = false;
bool startUDP = false;
bool done = false;
char input[256];

pthread_t clientThread = 0;

using namespace std;
using namespace service_if;

// Thread handler type declaration
void* MyHandler (void*);


static void my_sig_handler(int sig_num)
{
	if (sig_num == SIGINT || sig_num == SIGTERM)
	{
		done = true;
		input[0] = '\n';
	}
}


/*
 * A very simple client application used to test out the server.  It just captures input from the
 * command line and sends it to the server.
 */
int main (int argc, char* argv[])
{
    int portNo = 0;
    fstream* inFile = NULL;
    bool connected = false;
	int retVal = TcpService::EXIT_SUCCES;

	if (!SignalSetup(my_sig_handler))
	{
		cerr << "Signals not set up correctly\n";
		return TcpService::EXIT_FAIL;
	}

    if(argc < 4)
    {
        cerr<<"Usage : ./client <host name> <port> <-T|-U> [read file]"<<endl;
        return TcpService::EXIT_FAIL;
    }

    portNo = atoi(argv[2]);

    if((portNo > 65535) || (portNo < 2000))
    {
        cerr << "Enter a port number between 2000 - 65535" << endl;
        return TcpService::EXIT_FAIL;
    }

    if (strcmp(argv[3], "-T") == 0)
    {
    	startTCP = true;
    }
    else if (strcmp(argv[3], "-U") == 0)
    {
    	startUDP = true;
    }

    TcpService* service = NULL;
    if (startTCP)
    {
		service = new TcpService(TcpService::CONN_CLIENT, argv[1], portNo);

		if (service != NULL)
		{
			if (service->SetupClient())
			{
				connected = service->ClientConnect();
			}
		}
    }

    UdpService* udpService = NULL;
    if (startUDP)
    {
    	udpService = new UdpService(UdpService::CONN_CLIENT, argv[1], portNo);

    	if (udpService != NULL)
    	{
    		if (udpService->SetupSocket())
    		{
    			connected = true;
    		}
    	}
    }


    // Grab the contents of the file given as an argument
    // Since this is optional, we check for existence first
    if (argc >= 4)
    {
        char* fileCmd = argv[4];
        if (fileCmd != NULL)
        {
            inFile = new fstream(fileCmd);
        }
    }

    // We wait until the server says it's READY.
    int listenFd = 0;
    if (connected && startTCP)
    {
    	listenFd = service->ListenFd();
    }
    else if (connected && startUDP)
    {
    	listenFd = udpService->ListenFd();
    }
    while (!done && !ServerReady(listenFd));

    cout << "Server is ready.  Continuing..." << endl;

    if (pthread_create(&clientThread, NULL, MyHandler, &listenFd) < 0)
    {
    	perror ("Client thread creation error:");
    	retVal = TcpService::EXIT_FAIL;
    }

    // send stuff to server
    input[256] = { 0 };

    // Register a name with the server
    string name;

    cout << "Enter a Name: " << flush;
    cin >> name;

    string sendName = "Name:" + name;
    if (startTCP)
    {
    	if (write (listenFd, sendName.c_str(), sendName.length()) < 0)
    	{
    		perror ("Bad Name Register with TCP");
        	retVal = TcpService::EXIT_FAIL;
    	}
    }
    else if (startUDP)
    {
    	if (udpService->SendMsg(sendName) < 0)
    	{
    		perror ("Bad Name Register with UDP");
        	retVal = TcpService::EXIT_FAIL;
    	}
    }

    // If we provided a file, use that as input
    // otherwise, allow manual entry
    if (inFile != NULL)
    {
        // Using a file.  Grab contents
        cout << "Detected file.  Sending Contents..." << endl;
        while (inFile->getline(input, 256))
        {
            int len = strlen(input);
            if (len > 0)
            {
                cout << "Sending: [" << input << "]" << endl;
                if (startTCP)
                {
					//if (write(listenFd, input, len) < 0)
                	if (send (listenFd, (void*)input, len, MSG_DONTWAIT) < 0)
					{
						perror("client tcp send");
					}
                }
                else if (startUDP)
                {
                	string str(input);
                	if (udpService->SendMsg(str) < 0)
                	{
                		perror("error client udp write");
                	}
                }
                usleep(10000);
            }
        }
        cout << "Contents End.  Sending Exit To Server.";
        write(listenFd, "exit", 4);
        inFile->close();
        done = true;
    }
    else
    {
        // Manual entry
        while((strcmp(input, "exit") != 0) && !done)
        {
            cout << "Enter stuff: " << flush;
            bzero(input, 256);
            while (input[0] == '\0')
            	cin.getline(input, 256, '\n');

            if (strlen(input) > 0)
            {
				if (startTCP)
				{
					//if (write(listenFd, input, strlen(input)) < 0)
					if (send (listenFd, (void*)input, strlen(input), MSG_DONTWAIT) < 0)
					{
						perror("client tcp write");
					}
				}
				else if (startUDP)
				{
					string str(input);
					if (udpService->SendMsg(str) < 0)
					{
						perror("client udp write");
					}
				}
            }
        }
    }
    cout << "Bye. Bye." << endl;
    done = true;
    char* msg = new char[256];
    if (pthread_join (clientThread, (void**)&msg) < 0)
    {
    	pthread_kill (clientThread, 0);
    }
    else
    {
    	cout << msg << endl;
    }
    close(listenFd);
    delete(inFile);
    delete(service);
    return retVal;
}

bool ServerReady(int fd)
{
    bool ready = false;
    if (fd < 0)
    {
        cerr << "FD LT ZERO: " << fd << "\n";
        return false;
    }

    if (startUDP)
    {
    	return true;
    }

    char buf[100];
    if (recv(fd, buf, 100, 0) < 0)
    {
        perror ("recv from server");
        sleep(100);
        return false;
    }
    else
    {
        if (strncmp(buf, "READY", 5) == 0)
        {
            ready = true;
        }
    }
    return ready;
}

void* MyHandler(void* arg)
{
	int fd = *((int*)arg);
	int result = -1;
	fd_set readset;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	FD_ZERO(&readset);
	char buf[128];
	int bufLen = strlen(buf);
	memset((void*)&buf, 0, bufLen);
	string str("This thread has ended");

	printf ("Starting thread for FD: %d\n", fd);
	while (!done)
	{
		FD_SET(fd, &readset);
		result = select (fd + 1, &readset, NULL, NULL, &tv);
		if (result > 0 && (FD_ISSET(fd, &readset) > 0) && !done)
		{
			int len = recv (fd, (void*)buf, 128, MSG_DONTWAIT);
			buf[len] = '\0';
			if (len > 0)
			{
				printf("RECV : [%d]: %s\n", len, buf);
			}
			FD_CLR(fd, &readset);
		}
		memset((void*)&buf, 0, bufLen);
	}
	// Attempt to send an exit to server
	printf ("Exited Recv Loop. Sending EXIT to Server");
	send (fd, "exit", 4, MSG_DONTWAIT);
	return (void*)str.c_str();
}

bool SignalSetup(void (*handler)(int))
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


