#include <string.h>
#include <cstring>
//#include <unistd.h>
#include <stdio.h>
//#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

#include "TcpService.h"
#include "UdpService.h"

bool ServerReady(int fd);

// Using TCP or UDP
bool startTCP = false;
bool startUDP = false;

using namespace std;
using namespace service_if;

/*
 * A very simple client application used to test out the server.  It just captures input from the
 * command line and sends it to the server.
 */
int main (int argc, char* argv[])
{
    int portNo = 0;
    fstream* inFile = NULL;
    bool connected = false;

    if(argc < 4)
    {
        cerr<<"Usage : ./client <host name> <port> <-T|-U> [read file]"<<endl;
        return -1;
    }

    portNo = atoi(argv[2]);

    if((portNo > 65535) || (portNo < 2000))
    {
        cerr << "Enter a port number between 2000 - 65535" << endl;
        return -1;
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
    while (!ServerReady(listenFd));

    cout << "Server is ready.  Continuing..." << endl;

    // send stuff to server
    char input[256];

    // Register a name with the server
    string name;

    cout << "Enter a Name: ";
    cin >> name;

    string sendName = "Name:" + name;
    if (startTCP)
    {
    	if (write (listenFd, sendName.c_str(), sendName.length()) < 0)
    	{
    		perror ("Bad Name Register with TCP");
    	}
    }
    else if (startUDP)
    {
    	if (udpService->SendMsg(sendName) < 0)
    	{
    		perror ("Bad Name Register with UDP");
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
					if (write(listenFd, input, len) < 0)
					{
						perror("client tcp write");
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
    }
    else
    {
        // Manual entry
        while(strcmp(input, "exit") != 0)
        {
            cout << "Enter stuff: ";
            bzero(input, 256);
            cin.getline(input, 256);

            if (startTCP)
            {
				if (write(listenFd, input, strlen(input)) < 0)
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
    cout << "Bye. Bye." << endl;
    close(listenFd);
    delete(inFile);
    delete(service);
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


