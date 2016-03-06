#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>


bool ServerReady(int fd);


using namespace std;

/*
 * A very simple client application used to test out the server.  It just captures input from the
 * command line and sends it to the server.
 */
int main (int argc, char* argv[])
{
    int listenFd, portNo;
    struct sockaddr_in svrAdd;
    struct hostent *server;
    fstream* inFile = NULL;

    if(argc < 3)
    {
        cerr<<"Usage : ./client <host name> <port> [read file]"<<endl;
        return -1;
    }

    portNo = atoi(argv[2]);

    if((portNo > 65535) || (portNo < 2000))
    {
        cerr << "Enter a port number between 2000 - 65535" << endl;
        return -1;
    }

    // create client socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return -1;
    }

    server = gethostbyname(argv[1]);

    if(server == NULL)
    {
        cerr << "Host does not exist" << endl;
        return -1;
    }

    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;

    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

    svrAdd.sin_port = htons(portNo);

    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));

    if (checker < 0)
    {
        cerr << "Cannot connect!  No Server?" << endl;
        return -1;
    }

    // Grab the contents of the file given as an argument
    // Since this is optional, we check for existence first
    if (argc >= 4)
    {
        char* fileCmd = argv[3];
        if (fileCmd != NULL)
        {
            inFile = new fstream(fileCmd);
        }
    }

    // We wait until the server says it's READY.
    while (!ServerReady(listenFd));

    cout << "Server is ready.  Continuing..." << endl;

    // send stuff to server
    char input[256];

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
                if (write(listenFd, input, len) < 0)
                {
                    perror("client write");
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

            if (write(listenFd, input, strlen(input)) < 0)
            {
                perror("client write");
            }
        }
    }
    cout << "Exiting.." << endl;
    close(listenFd);
    delete(inFile);
}

bool ServerReady(int fd)
{
    bool ready = false;
    if (fd < 0)
    {
        cerr << "FD LT ZERO: " << fd << "\n";
        return false;
    }

    char buf[100];
    if (recv(fd, buf, 100, 0) < 0)
    {
        perror ("recv from server");
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


