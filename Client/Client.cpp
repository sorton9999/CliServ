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

using namespace std;

/*
 * A very simple client application used to test out the server.  It just captures input from the
 * command line and sends it to the server.
 */
int main (int argc, char* argv[])
{
    int listenFd, portNo;
    bool loop = false;
    struct sockaddr_in svrAdd;
    struct hostent *server;
    string filename = "";
    ifstream *fs = NULL;
    ostringstream s_str;
    istringstream i_str;

    if(argc < 3)
    {
        cerr<<"Usage : ./client <host name> <port> [filename]"<<endl;
        return 0;
    }
    else if (argc == 4)
    {
    	filename = argv[(argc-1)];
    	fs = new ifstream(filename.c_str());

//    	int n = 5;
//    	if (filename.find_last_of("2") != string::npos)
//    	{
//    		while (n > 0)
//    		{
//    			cout << "." << flush;
//    			sleep(1);
//    			--n;
//    		}
//    	}
    }

    portNo = atoi(argv[2]);

    if((portNo > 65535) || (portNo < 2000))
    {
        cerr<<"Enter a port number between 2000 - 65535"<<endl;
        return 0;
    }

    // create client socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }

    server = gethostbyname(argv[1]);

    if(server == NULL)
    {
        cerr << "Host does not exist" << endl;
        return 0;
    }

    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;

    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

    svrAdd.sin_port = htons(portNo);

    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));

    if (checker < 0)
    {
        cerr << "Cannot connect!" << endl;
        return 0;
    }

    // send stuff to server
    for(;;)
    {
        string s = "";

        if ((filename.length() > 0) && fs->is_open())
        {
        	while (!fs->eof())
        	{
        		(*fs) >> s;
        		write(listenFd, s.c_str(), s.length());
        		s = "";
        		sleep(1);
        	}
        	write(listenFd, "exit", 4);
        	cout << "Auto Exit" << endl;
        	break;
        }
        else
        {
        	cout << "Enter stuff: ";
        	getline(cin, s);
            write(listenFd, s.c_str(), s.length());
        }
    }
    cout << "Exiting.." << endl;
    close(listenFd);
    fs->close();
    delete fs;
    fs = NULL;
}


