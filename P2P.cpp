// P2P.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <windows.h>
#include <winsock.h>
#include <ws2tcpip.h>
#else
// unix includes here
#endif

#include <iostream>
#include <fstream>


//#include <netinet/in.h>

#include "stdafx.h"
using namespace std;

#define DEFAULT_PORT 5556
#define DEFAULT_ADDR "127.0.0.1"

#define SOCK_STREAM      1
#define SOCK_DGRAM      2
#define SOCK_RAW      3
#define AF_INET      2 
#define IPPROTO_TCP      6



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, _TCHAR* argv[])
{

	if (WIN32_LEAN_AND_MEAN) {
		int result;
		//initialising winsock
		WSADATA WsaData;
		if(result = WSAStartup(MAKEWORD(2,2),&WsaData) != 0)
		{
			cout<<"WSAStartup Failed!";
			return 1;
		}
	}
	
	unsigned long ulAddr = INADDR_NONE;


	SOCKET sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	

	if (sck == INVALID_SOCKET)
	{
		cout<<"Socket Initialising Failed!";
		int k;
		cin>>k;
		return false; //Don't continue if we couldn't create a //socket!!
	}

	SOCKADDR_IN addr;// The address structure for a TCP socket	
	addr.sin_family = AF_INET; // Address family
	addr.sin_port = htons(DEFAULT_PORT);   // Assign port to this socket
	addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDR); //INADDR_ANY; 

	/*ulAddr = 
	addr.sin_addr.s_addr = ulAddr;*/

	if (bind(sck, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		//We couldn't bind (this will happen if you try to bind to the same  
		//socket more than once)
		cout<<"Socket Binding Failed!";
		int k;
		cin>>k;
		return false;
	}

	//Now we can start listening (allowing as many connections as possible to  
	//be made at the same time using SOMAXCONN).
	listen(sck, SOMAXCONN);

		

	socklen_t sin_size;
	int sockfd = sck,new_fd;// listen on sock_fd, new connection on new_fd

	struct sockaddr_storage their_addr; // connector's address information
	char s[INET6_ADDRSTRLEN];

	while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("tracker: got connection from %s\n", s);

        //if (!fork()) { // this is the child process
            closesocket(sockfd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            closesocket(new_fd);
            exit(0);
        //}
        closesocket(new_fd);  // parent doesn't need this
    }

	int r;
	cin >> r;
	return 0;
}

