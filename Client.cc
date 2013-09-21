#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h> //inet_addr
#include <unistd.h> //close

#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include <map>

#include <vector>

#include <string>

#include "clientItem.hh"

#include "connect_library.cc"

#include "util.hh"

#include <fstream>
#include <iostream>
using namespace std;




#define PORT "3490"
#define HOST "localhost"

#define PORT_ACCEPT 5555
#define PORT_CONSOLE 6666

#define MAXLEN 1024

#define MAX_CONN 10

#define MAX_LISTEN 10


#define SECOND 1000
#define TIMEOUT (30 * SECOND)

typedef struct conexion
{
   bool primero;
   char buf[1025]; 
   const char* filename; 
   FILE *file;   
   int off; 
   int sent;
   int rval;
   bool leer;
   int total;
} conexion;

client * this_is_me;

int new_console_pos (struct pollfd fds[MAX_CONN],int nfds, int fd)
{
  int i;
  int pos = -1;
  bool encontre = false;  
  for (i = 0; (i < nfds) && (!encontre); i++)
  {
  if (fds[i].fd == fd)
  {
    encontre = true;
    pos = i;
  }
  }
  return pos; 
} 

void get_all_buf2(int sock, std::string & inStr, int &totalSize) {
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

void processPeerToPeer(int port_accept,int port_console,int serv_socket) {
	
   conexion * info;
   map<int, conexion *> conexions; 
   
   map<int,string> archivos;
   archivos[6667] = "Love.mp3";
   archivos[6668] = "Mendeley.exe";
   archivos[6669] = "This.avi.exe";
   
   map<int,string> archivos_rec;
   archivos_rec[6667] = "Love_rec.mp3"; 
   archivos_rec[6668] = "Mendeley_rec.exe";  
   archivos_rec[6669] = "This.avi_rec.exe"; 	
   
   bool eliminar = false;
   bool order_fds = false;
   int console,console_fds;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i;                                //for loops
   char buff[MAX_BUFF_SIZE];
   char buff2[MAX_BUFF_SIZE], buff3[MAX_BUFF_SIZE];              //for sending and recieving text
   struct sockaddr_in their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;                             //size of data recieved
   int socket_client;
   console = -1;
   order_fds = false;          


    //Creo entrada[0] de my_fds con el socket de la conexión con el cliente
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_accept);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;


	//Creo la entrada[1] de my_fds con el socket de la conexión con la consola
	curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_console);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[1] = *curr;


	//Creo la entrada[2] de my_fds con el socket de la conexión con el tracker
	curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = serv_socket;
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[2] = *curr;


    //Inicializo la cantidad de fds en 3
    num_fds = 3;

    // buff = (char*)malloc(MAX_BUFF_SIZE);         
     string auxConsole="";

     while (num_fds < MAX_CONN)
     {
     
         //reordeno my_fds
         if (order_fds)
         {
	          order_fds = false;     
	          num_fds = order(my_fds, num_fds);
	          cout << "ordeno" << "\n";
	          if (console != -1)
	            console = new_console_pos(my_fds,num_fds,console_fds);
         }         
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
				        //seguir ver receive
				        //Accept the connection
				        sin_size = sizeof their_addr;
				        new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
				        new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
				        
				        if (new_conn->fd < 0)
				        {
				            perror("  accept() failed");
				            //end_server = TRUE;
				        }     
				        
						new_conn->events = POLLOUT;
						new_conn->revents = 0;
						
						info = new conexion;
						conexions[new_conn->fd] = info;
					    info->primero = true;  
					    info->off = 0;
					    info->leer = true;	

				        //Add it to the poll call
				        my_fds[num_fds] = *new_conn;
				        num_fds++;
    			 }
    			 
    			 if ((i == 1) && (curr->revents != 0))
    			 {
    					printf("Inicio consola\n\r");

    					//Accept the connection
    					sin_size = sizeof their_addr;
    					new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
    					new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
    					new_conn->events = POLLIN;
    					new_conn->revents = 0;
    					
    					cout << "Agrego el socket de la consola a las posición " <<  num_fds << "\n";					
    					
    					//Add it to the poll call
    					my_fds[num_fds] = *new_conn;
    					console = num_fds;
    					
    					num_fds++;
    			 }


    			 if ((i == 2) && (curr->revents != 0)) {
					printf("Nueva respuesta del tracker\n\r");    		

					char data[MAX_MSG_SIZE];
                	int r_data_size = recv(my_fds[2].fd, data, MAX_MSG_SIZE, 0);
                	data[r_data_size]='\0';
                	cout<<"El tracker responde:"<< data<<"\n";
    			 }
    			 
    				

    			 if ((i == console) && (curr->revents != 0))
    			 {                       	             

	    		  char data[MAX_BUFF_SIZE];
		          int size = recv(curr->fd, data, MAX_BUFF_SIZE, 0);
		          data[size]='\0';
		          string out(data);

		          auxConsole = auxConsole + out;

	              if (size <=0){
	                 printf("Console size<=0, algun error\n");
	                 eliminar = true;
	                 order_fds = true;
	              }
	              else if (strchr(data,'\n')!=0) {
	              		  
						  auxConsole = auxConsole.size() > 2 ? auxConsole.substr(0, auxConsole.size()-2) : auxConsole;

        				  splitstring s(auxConsole);        				          				          				 

		                  vector<string> splitV = s.split(' ',1);

			              string command = splitV[0];

			              string toSend="";
        				  
		                  if (command.find("show") != std::string::npos) { //show command
		                    string param =splitV.size() > 1 ? splitV[1] : "";
		                    //cout <<"param:#"<<param<<"#\n";
		                    if (param.size() > 0) {

		                      if (param.find("share") != std::string::npos) { //"show share" muestra todos los archivos compartidos y los bytes transmitidos
		                          print_shared_files(this_is_me);
		                      }
		                      else if (param.find("downloads") != std::string::npos) { //"show downloads" muestra todas las descargas actuales para este cliente, junto con la dir del uploader y bytes descargados
		                          print_downloads(this_is_me);
		                      }
		                      else if (param.find("uploads") != std::string::npos) { //"show uploads" muestra todas las cargas en progreso, junto con la dirección del downloader y los bytes entregados
		                          print_uploads(this_is_me);
		                      }
		                      else perror("Invalid argument for show command");                        


		                    }
		                    else perror("Not enough arguments, need to specify what to show");                        
		                  }

		                  if (command.find("share") != std::string::npos) { //share command			                    
		                    //cout <<"console command:"<<splitV.size()<<"-"<<command<<"\n";
		                    string file =splitV.size() > 1 ? splitV[1] : "";

		                    if (file.size() > 0) {
		                      
		                      cout << "file to share:@"<<file<<"@\n";
		                      share_file(this_is_me,file);      
		                      
		                      toSend = "PUBLISH\n"+file+"\n"+getFileMD5(this_is_me,file)+"\r\n";
		                    }
		                    else perror("Not enough arguments, need to specify file to share");                        
		                  }
		        				  

		                  if (command.find("download") != std::string::npos) { //download command
	                   	 	string param =splitV.size() > 1 ? splitV[1] : "";		                   
		                    if (param.size() > 0) {		                                           
		                    	string file = param;
		                    	toSend = "SEARCH\n"+file;		                    	
		                    	
		                    	//falta agregar cosas
			                    info = new conexion;
								conexions[new_conn->fd] = info;
							    info->primero = true;  
							    info->off = 0;
							    info->leer = true;
							    info->total = 0;	
		                    }
		                    else perror("Not enough arguments, need to specify what to show");    
		                  }

		                  if (auxConsole.compare("quit")==0) {         
		                    //delete this_is_me;   
		                    cout << "Cerrando cliente.. \n";
		                    exit(0);
		                  }
                                              
        				  // en out queda gurdado lo que recibo por telnet
        				  				  
        				  //envio al servidor lo que me llego por telnet				

        				  //char *toSend = (char*)auxConsole.c_str();
        				  if (toSend.size() > 0)
        				  	send(serv_socket, toSend.c_str(), toSend.size(), 0);

              			           
		                  auxConsole = "";		              
	              }

			   }
    			   
			  if ((i != console) && (i > 2) && (curr->revents != 0))
			   {
				  //nuevo  
					info = conexions[curr->fd];
				  //nuevo   
				  cout << "revents cliente " << curr->revents << "\n"; 
				  if ((curr->revents != POLLIN) && (curr->revents != POLLOUT)) 
				  {
					  printf("algun error\n");
					  eliminar = true;
					  order_fds = true;; 
				  }
				  else
				  {
	
					if (curr->revents == POLLIN) 
					{							  
						  if (info->primero)
						  {
							    cout << "primero rec" << "\n";
							    info->primero = false;							    
							    info->filename = archivos_rec[port_console].c_str();
							    info->file = fopen(info->filename, "wb"); 
							    if (!info->file)
							    {
									cout << info->filename << cout << " No se pudo abrir el archivo para escribir" << "\n";
							        eliminar = true;
							    }
							    else
							    {
									cout << info->filename << cout << " Se abrio el archivo para escribir" << "\n";
								}
						  }
						  if (!eliminar)
						  {
						        info->rval = recv(curr->fd, info->buf, sizeof(info->buf), 0);
						        cout << info->filename << cout << " invoco receive " << "\n";
						        if (info->rval < 0)
						        {
									   if ((errno != EWOULDBLOCK) && (errno != EAGAIN))
									   {
										cout << info->filename << cout << " Can't read from socket "<< "\n"; 
										cout << info->filename << cout << " error es "<< errno << "\n";   
							            fclose(info->file);
							            eliminar = true;						   
									   }	
						        }
						        else
						        {	
									cout << info->filename << cout << " recibi " << info->rval << " bytes" << "\n";
									info->total += info->rval;
									cout << info->filename << cout << " total recibido " << info->total << " bytes" << "\n";						
							        if (info->rval == 0)
							        {
							            eliminar = true;
							            fclose(info->file);
							            cout << info->filename << cout << " recibi todo " << "\n";	
									}
									else
									{
										info->off = 0;
								        while (info->off < info->rval)	
								        {
								            int written = fwrite(&(info->buf)[info->off], 1, info->rval - info->off, info->file);								            
								            if (written < 1)
								            {
												cout << info->filename << cout << " Can't write to file" << "\n";
								                fclose(info->file);
								                eliminar = true;
								            }
								            else
								            {
											cout << info->filename << cout << " escribi " << written << " bytes" << "\n";	
								            info->off += written;
											}
								        }									 
									}
								}
							}
													       
					  }
					  else
					  {
						  
						  if (info->primero)
						  {
							    cout << "primero" << "\n";
							    info->primero = false;
							    info->filename = archivos[port_console].c_str();
							    info->file = fopen(info->filename, "rb"); 
							    if (!info->file)
							    {
									cout << info->filename << cout << " Can't open file for reading" << "\n";
							        eliminar = true;
							    }
							    else
							    {
									cout << info->filename << cout << " abri archivo para leer" << "\n";
								}
						  }
						  if ((!eliminar) || (!feof(info->file))) 
						  {
								  if (info->leer) 
								  { 
									  if (feof(info->file)){
										  	cout << info->filename << cout << " fin archivo" << "\n";
								            fclose(info->file);
											eliminar = true;
										  
									  }
									  else { 
								        info->rval = fread(info->buf, 1, sizeof(info->buf), info->file); 
								        if (info->rval < 1)
								        {
											cout << info->filename << cout << " Can't read from file " << "\n";
								            fclose(info->file);
											eliminar = true;
								        }
								        else
								        {
									        info->off = 0;
									        info->leer = false;
									        cout << info->filename << cout << " lei " << info->rval << " bytes" << "\n";	  
										}
									}
								   }
								   if (!eliminar)
								   {	
							           info->sent = send(curr->fd, &(info->buf)[info->off], info->rval - info->off, 0);
							           if (info->sent < 0)
							           {
										   if ((errno != EWOULDBLOCK) && (errno != EAGAIN))
										   {
											cout << info->filename << cout << "Can't write to socket"<< "\n";
											cout << info->filename << cout << "error es"<< errno << "\n"; 
											eliminar = true;	 
							                fclose(info->file);					   
										   }
									   }
									   else
									   {
										   cout << info->filename << cout << " envie " << info->sent << "bytes" << "\n";
										   info->total += info->sent;
										   cout << info->filename << cout << " voy enviando " << info->total << "bytes" << "\n";		
										   info->off += info->sent;
										   if (info->off >= info->rval){	
										     cout << info->filename << cout << " 0ff " << info->off << "\n";
										     cout << info->filename << cout << " rval " << info->rval << "\n";			   
										     info->leer = true;
										   }
									   }
								   }						   			  
						  }
						  else
						 {
							 fclose(info->file);
							 eliminar = true;							 
						 }	  
					  }			
				  } 	
				if (eliminar)
				{	 
					conexions.erase(curr->fd);	 
				    delete (info);					  
				}					  
		  		   
			   }
			     
		 if (eliminar)
		 {	   
			   cout << info->filename << cout << " elimino pos " << i <<"\n";	
			   close (curr->fd);	   
			   curr->fd = -1;
			   order_fds = true;
			   
		 }
		 eliminar = false;	
         }
     }
}



#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT "6666"


char* myip=(char*)"";
char* acceptport=(char*)"";
char* consoleport=(char*)"";
char* trackerIp=(char*)"";
char* trackerPort=(char*)"";

bool readProperties(const char * propfile) {
	ifstream infile(propfile);
		
	if (infile.good()) {
		printf("good file\n");
		//infile.open();
		string line;
		vector<string> propValues;	
		while (std::getline(infile, line)){			
		    splitstring s(line);
		    vector<string> sV = s.split('=',1);
			string paramKey = sV[0];
			string paramVal = sV[1];
			propValues.push_back(paramVal);
		}

		infile.close();

		

		myip=(char*)propValues.at(0).c_str();
		myip[propValues.at(0).size()-1] = '\0';

		acceptport=(char*)propValues.at(1).c_str();
		acceptport[propValues.at(1).size()-1] = '\0';

		consoleport=(char*)propValues.at(2).c_str();
		consoleport[propValues.at(2).size()-1] = '\0';

		trackerPort=(char*)propValues.at(3).c_str();
		trackerPort[propValues.at(3).size()-1] = '\0';

		trackerIp=(char*)propValues.at(4).c_str();
		trackerIp[propValues.at(4).size()-1] = '\0';


		return true;
	}
	else {
		printf("not good file\n");
		return false; 	
	}
}

char* Concatenate(const char* first, const char* second)
{
  char* mixed = new char[strlen(first) + strlen(second) + 2 /* for the ': ' */ + 1 /* for the NULL */];
  strcpy(mixed, first);
  strcat(mixed, ": ");
  strcat(mixed, second);

  return mixed;
}

int main(int argc, char *argv[])
{
    int sockfd,port_accept,port_console;

    if (!readProperties("client.properties")){
	    
	    if (argc < 4) {
	       fprintf(stderr,"No property file found, must provide parameters:  %s hostname acceptport consoleport\n", argv[0]);
	       exit(0);
	    }	  
    	
    	myip = argv[1];				
		acceptport = argv[2];			
		consoleport = argv[3];	

    	trackerPort = (char*)TRACKER_PORT;		
    	trackerIp = (char*)TRACKER_IP;					
		
		if (argc > 4)
			trackerPort = argv[4];		
		
		if (argc > 5)
			trackerIp = argv[5];
	
	}
	
	printf("myip=%s\naccept=%s\nconsole=%s\ntrackerport=%s\ntrackerip=%s\r\n",myip,acceptport,consoleport,trackerPort,trackerIp);

	printf("Comienza cliente \n");
	printf("Me conecto al servidor \n");

    this_is_me = client_create(myip, acceptport);

    //connect to tracker
    sockfd = connect_socket(trackerIp,trackerPort);    
    
    const char* s1 = myip;
    const char* s2 = acceptport;
    const char* sep =":";
    char msg[MAX_BUFF_SIZE];        
	sprintf(msg, "NEWCLIENT\n%s:%s", myip,acceptport);
	printf("%s\n", msg);    
    send(sockfd, msg, sizeof(msg), 0);    
    
    port_accept = (int)strtol(acceptport,NULL,10);
    port_console = (int)strtol(consoleport,NULL,10);
	

	processPeerToPeer(port_accept,port_console,sockfd);

   
}
