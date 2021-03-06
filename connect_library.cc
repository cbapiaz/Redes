#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


#include <poll.h>

#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>


#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <unistd.h> //close

#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include <string>
#include <iostream>
using namespace std;

#define MAXLEN 1024

#define MAX_LISTEN 10
#define MAX_CONN 10

#define SECOND 1000
#define TIMEOUT (30 * SECOND)

static void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static int connect_socket(const char* host, char* port)
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    return sockfd;
}


static int listen_socket(int port)
{
    struct sockaddr_in a;
    int s;
    int yes;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
            (char *) &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(s);
        return -1;
    }
    memset(&a, 0, sizeof(a));
    a.sin_port = htons(port);
    a.sin_family = AF_INET;
    if (bind(s, (struct sockaddr *) &a, sizeof(a)) < 0) {
        perror("bind");
        close(s);
        return -1;
    }    
    int res = listen(s, MAX_LISTEN);
    if (res < 0)
	{
		perror("listen");
		close (s);
		return -1;
	}
	int on = 1;
	res = ioctl(s, FIONBIO, (char *)&on);
    if (res < 0)
	{
		perror("ioctl");
		close (s);
		return -1;
	}	
	printf("Aceptando conexiones en el puerto %d\n", port);
    return s;
}

static int order(struct pollfd fds[MAX_CONN], int nfds)
{
  int i,j;
  for (i = 0; i < nfds; i++)
  {
	if (fds[i].fd == -1)
	{
	  printf("Hay una conexion menos\n\r"); 	
	  for(j = i; j < nfds; j++)
	  {
		fds[j].fd = fds[j+1].fd;
	  }
	  nfds--;
	}
  }
  return nfds;
} 

#define MAX_BUFF_SIZE 1000

static void get_all_buf(int sock, std::string & inStr, int &totalSize) {
    int n = 1, total = 0, found = 0;
    char c;
    char temp[1024*1024];

    // Keep reading up to a '\n'
    while (!found) {
        n = recv(sock, &temp[total], sizeof(temp) - total - 1, 0);
        if (n == -1) {
            /* Error, check 'errno' for more details */
            break;
        }
        total += n;
        temp[total] = '\0';
        found = (strchr(temp, '\n') != 0);
    }

    totalSize = total-2;
    inStr = temp;
    inStr = inStr.substr(0, inStr.size()-2); //all characters except the line break
}
