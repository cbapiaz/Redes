#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

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


#include "util.hh"
#include "clientItem.hh"

#include <map>

#include <vector>

#include <string>

#include <sstream>


#include <openssl/md5.h>

#include <iostream>

using namespace std;


#include "connect_library.cc"

#define HOST "localhost"

#define PORT_ACCEPT 5555
#define PORT_CONSOLE 6666

#define MAXLEN 1024

#define MAX_CONN 10

#define MAX_LISTEN 10

#define SECOND 1000
#define TIMEOUT (30 * SECOND)

void print_clients (map<int, trackerClient*> m){
	cout << "Los clientes registrados son \n";
    for(std::map<int, trackerClient*>::const_iterator it = m.begin(); it != m.end(); it++)
    {
    	cout << it->first <<  "\n"; ;
    	//Do something
    }
}

void pollserver(int port_accept) {   
  
   bool order_fds;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i, j;                                //for loops
   char buff[255], buff2[255];              //for sending and recieving text
   struct sockaddr_in my_addr, their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;
   string ip;                             //size of data recieved
   string port; 
   order_fds = false;   
   
   map<int, trackerClient*> clients;  
   bool eliminar = false;
    
    //Creo entrada[0] de my_fds con el socket de la conexión con el cliente
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_accept);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;


    //Inicializo la cantidad de fds en 2
    num_fds = 1;

     while (num_fds < MAX_CONN)
     {
		 //reordeno my_fds
		 
		 num_fds = order(my_fds, num_fds);
     
         //reset all event flag
         for (i = 1; i < num_fds; i++)
         {
            curr = &my_fds[i];
            curr->events = POLLIN ;
            curr->revents = 0;
         }         
         
         printf("calling poll (%d sockets)\n\r", num_fds);
         if (poll(my_fds, num_fds, -1) == -1)
         {
            perror("poll");
            exit(0);
         }
         
        for (i = 0; i < num_fds; i++)
        {
			 curr = &my_fds[i];	
 		 
			 if ((i == 0) && (curr->revents != 0))
			 {
				printf("Hay nuevo cliente\n\r");

				//Accept the connection
				sin_size = sizeof their_addr;
				new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
				new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
				
		        if (new_conn->fd < 0)
		        {
		            perror("accept");
		        }				
				
				new_conn->events = POLLIN;
				new_conn->revents = 0;
                cout<<"accept the connection to the new client\n";
				// leo el mensaje del cliente
                string msg;              
                int size ;
                
                char data[MAX_MSG_SIZE];
                int r_data_size = recv(new_conn->fd, data, MAX_MSG_SIZE, 0);
                data[r_data_size]='\0';

                msg = string(data);

                //get_all_buf(new_conn->fd,msg,"\r\n",size);                  
                cout<<msg<<"\n";

                splitstring s(msg);
                vector<string> splitV = s.split('\n',1);
                string command = splitV[0];

                /**  definir el protocolo aca segun el comando **/

                if (command.find("NEWCLIENT") != std::string::npos) { //show command
                	char buff[MAX_BUFF_SIZE];                	

                    if (splitV.size() < 2) {
                    	const char *errMsg= "NEWCLIENT command error, not enough parameters";
                        perror(errMsg);                        
                        sprintf(buff, "fail\n%s\r\n", errMsg);    				
                    }
                    else {
                        vector<string> _sV = splitstring(splitV[1]).split(':',1);
                        if (_sV.size() < 2) {
                        	const char *errMsg= "NEWCLIENT command error, invalid parameter format must be <IP>:<PORT>";
                            perror(errMsg);
                     		sprintf(buff, "fail\n%s\r\n", errMsg);    				       
                        }
                        else {
                            string ip = _sV[0];
                            string port = _sV[1];

            				//Add it to the poll call
            				my_fds[num_fds] = *new_conn;
            				num_fds++;	            			
            				
            				addNewTrackerClient(clients,new_conn->fd,ip,port);
                            //clients[new_conn->fd] = client_create(ip,port);
            				print_clients(clients);
            				sprintf(buff, "ok");
                        }
                    }

                    send(new_conn->fd, buff, strlen(buff), 0);
                }
                else {
                    cout<<"nuevo mensaje: "<< msg;
                }
                
			 }

			 else if ((i > 0) && (curr->revents != 0))
			   {
				  
				  if (curr->revents != POLLIN) 
				  {
					  printf("REVENTS algun error\n");
					  eliminar = true;
				  }
				  else
				  {
					  printf("Recibo pedido cliente\n"); 
					  //buff_sz = recv(curr->fd, &buff, 254, 0);					  					  

                	  char data[MAX_MSG_SIZE];
		              int buff_sz = recv(curr->fd, data, MAX_MSG_SIZE, 0);
		              data[buff_sz]='\0';


					  if (buff_sz <= 0)
					  {
						  printf("pedido del cliente no valido error\n");
						  eliminar = true;
					  }
					  else
					  {  					  
			  				
			              string msg = string(data);

                          /*cout<<"New message from: "<<client_getcIp(clients[curr->fd])<<"@"<<client_getcPort(clients[curr->fd])<<"\n";
			              cout<<msg<<"\n";*/

			              splitstring s(msg);
			              vector<string> splitV = s.split('\n',1);
			              string command = splitV[0];

			              /**  definir el protocolo aca segun el comando **/
			              //publish a new file
			              if (command.find("PUBLISH") != std::string::npos) { //show command                            
                                char buff[MAX_BUFF_SIZE];                   

                                if (splitV.size() < 3) {
                                    const char *errMsg= "PUBLISH command error, not enough parameters";
                                    perror(errMsg);                        
                                    sprintf(buff, "fail\n%s\r\n", errMsg);                  
                                }
                                else {
                                    string fileName = splitV[1];
                                    string md5 = splitV[2];
                                    
                                    string res = publish_file(clients[curr->fd],fileName,md5);
                                    if (res=="") {
                                        sprintf(buff, "ok");
                                    }
                                    else sprintf(buff, "fail\n%s\r\n",res.c_str());
                                }

                                send(curr->fd, buff, strlen(buff), 0);
			              }

			              if (command.find("SEARCH") != std::string::npos) { //show command
                                char buff[MAX_BUFF_SIZE];                   

                                if (splitV.size() < 2) {
                                    const char *errMsg= "SEARCH command error, not enough parameters";
                                    perror(errMsg);                        
                                    sprintf(buff, "fail\n%s\r\n", errMsg);                  
                                }
                                else {

                                    string fileName = splitV[1];                                    
                                    string result = search_file(clients,fileName);

                                    sprintf(buff, "%s",result.c_str());
                                }

                                send(curr->fd, buff, strlen(buff), 0);
			              }			              						 
						  
					  }
				  }
			   }
		 if (eliminar)	   
			   curr->fd = -1;
		 eliminar = false;	
         }
     }
}

int main(int argc, char *argv[])
{
   char buff[255], buff2[255];              //for sending and recieving text
   struct sockaddr_in my_addr, their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;                             //size of data recieved	
	
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);     
     
     pollserver(portno);
         
}

