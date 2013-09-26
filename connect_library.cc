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

#include <sstream>

#include <vector>
using namespace std;

#define MAXLEN 1024

#define MAX_LISTEN 10
#define MAX_CONN 10

#define SECOND 1000
#define TIMEOUT (30 * SECOND)

#define MAX_MSG_SIZE 1024*1024


static void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static void getPeerInfo (int s, std::string &ip, std::string &port)
{
	ip = "";
	port = "";
	struct sockaddr_in their_addr;
	socklen_t addr_len = sizeof(their_addr);
	int err = getpeername(s, (struct sockaddr *) &their_addr, &addr_len);
	if (err != 0) {
	   error("getPeerInfo");	   
	}
	else{
	    ip = inet_ntoa(their_addr.sin_addr);
	    int p = (int) ntohs(their_addr.sin_port);    
	    
	    ostringstream ss;
	    ss << p;
	    port = ss.str();
	}	
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
	//setNonBlocking(s);
    
	printf("Aceptando conexiones en el puerto %d\n", port);
    return s;
}

static int setNonBlocking(int fd) {
    int on = 1;
    int res = ioctl(fd, FIONBIO, (char *)&on);

    if (res < 0)
    {
        perror("ioctl");
        close (fd);
        return -1;
    }   

    return res;
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
		fds[j].events = fds[j+1].events;
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

static vector<std::string> parseResponse (std::string s,std::string &md5)
{
    vector<std::string> vect;
    int i = 0;
    bool prim = true;
	std::string delimiter = "\n";
	
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
	    token = s.substr(0, pos);
	    if (i == 1){
			md5 = token;
		}
		if (i > 1)
		{
			if (token.size() > 0)
				vect.push_back (token);
		}
	    std::cout << "el token es" << token << std::endl;	  
	    s.erase(0, pos + delimiter.length());
		i++;
	}
	for (int i = 0; i < vect.size(); i++)
	{
		std::cout << vect[i] << std::endl;
	}
	//vect.push_back (token);
	std::cout << s;
	return vect;	  
}

static void choosePeer( vector<std::string> list, std::string &ip, std::string &port)
{
	int number = (rand () % (list.size()));
	std::cout << "random es " << number << std::endl;	
	std::string s = list[number];
	int pos = s.find (":");
	ip = s.substr(0,pos);
	port = s.substr(pos+1);

}

