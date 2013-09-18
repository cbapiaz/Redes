#include<stdio.h>
#include<stdlib.h>
#include <string>

#include <vector>
#include <map>

#include "clientItem.hh"

#include <iostream>
using namespace std;



#define MAX_FILES 1500

#define MAX_PEERS 500

typedef struct fileDescriptor
{
  	string name;
  	string md5;
  	int fd;
  	unsigned long size;
  	unsigned long bytes_transfered;
} fileDescriptor;


// A peer is another client connected to me
typedef struct peer {
	string ip;
	string port;
	vector<fileDescriptor> peer_files; // files being "shared" , currently being downloaded or uploaded
} peer;

//define the client structure
typedef struct client
{
        	string ip;
			string port;
			vector<peer> uploaders; // store information about the "clients" that i am uploading files to
			vector<peer> downloaders; // store information about the "clients" that i am downloading files from
			vector<fileDescriptor> shared_files; // file that i share with other clients or peers
} client;


#define BASE_DIR "share/"
/** create a new client */
client * client_create (string ip, string port)
{
	client* cli = new client;
	cli->ip = ip;
	cli->port = port; 	
	return cli;
}

/** get client ip */
string client_getcIp (client *cli){
	return cli->ip;
}

/** get client port */
string client_getcPort (client *cli){
	return cli->port;
}

/** destroy client */
void client_destroy (client *cli){
	delete cli;
} 

string search_file(map<int,client*> &clients,string file) {
	string md5;
	string clientsWithTheFile="";
	bool foundAny = false;

	for(std::map<int, client*>::const_iterator it = clients.begin(); it != clients.end(); it++)
    {
    	client * cli = it->second;
    	int k=0;
    	bool found = false;

    	while (!found && k < cli->shared_files.size()) {

    		found = (cli->shared_files[k].name.compare(file)==0);
    		k++;
    	}

    	if (found) {
    		fileDescriptor _fd =cli->shared_files[k];
    		md5 = _fd.md5;
    		
    		clientsWithTheFile += cli->ip + ":" + cli->port+"\n";
    	}

    	foundAny = foundAny || found;
    }

    if (foundAny) {
    	return "FILE\n"+md5+"\n"+clientsWithTheFile+"\n";
    }
    else return "fail\nempty";
}


void publish_file(client *cli,string file,string _md5) {
	if (cli != NULL) {		 		 
	     fileDescriptor fdesc;
	     fdesc.name = file;			
		 fdesc.md5 = _md5;
		 fdesc.fd = -1;
		 fdesc.size = -1;
		 fdesc.bytes_transfered = 0;
		 cli->shared_files.push_back(fdesc);		 
	}
	else perror ("share_file: client should not be null");
}

void share_file(client *cli,string file) {
	if (cli != NULL) {		 
		 int fd; unsigned long size;
		 string _md5 = getMD5(BASE_DIR + file,fd,size);

   	     if (_md5.size() > 0) {
   	     	 fileDescriptor fdesc;
   	     	 fdesc.name = file;			
			 fdesc.md5 = _md5;
			 fdesc.fd = fd;
			 fdesc.size = size;
			 fdesc.bytes_transfered = 0;

			 cli->shared_files.push_back(fdesc);

		 }
		 else {
		 	perror("\nCannot share file, error calculating md5\n");
		 }
	}
	else perror ("share_file: client should not be null");
}

void print_files(vector<fileDescriptor>& v) {
	for (int i=0;i<v.size();i++) { //
   	  cout<< "File: '"<<v[i].name << "' - MD5: "; print_md5_sum((unsigned char*)v[i].md5.c_str());cout<<" - Bytes: " << v[i].bytes_transfered << "\n";
   }
}
void print_shared_files(client * cli){
	cout<<"\nShared files\n";
	print_files(cli->shared_files);	  
}

//Muestra las descargas en progreso junto con la dirección del uploader y los bytes descargados.
void print_downloads(client * cli){
   cout<<"\nDownlading files\n";
   for (int i=0;i<cli->downloaders.size();i++) {
   	  cout<<"Downlading from: "<< cli->downloaders[i].ip << "@" << cli->downloaders[i].port <<"\n";
   	  cout<<"--------------------------------------------------------------------------------------\n";
 	  print_files(cli->downloaders[i].peer_files);
 	  cout<<"--------------------------------------------------------------------------------------\n"; 	     	  
   }
}

//Muestra las cargas en progreso junto con la dirección del downloader y los bytes entregados.
void print_uploads(client * cli){
   cout<<"\nUploading files\n";
   for (int i=0;i<cli->uploaders.size();i++) {
   	  cout<<"Uploading To: "<< cli->uploaders[i].ip << "@" << cli->uploaders[i].port <<"\n";  	  
   	  cout<<"--------------------------------------------------------------------------------------\n";
   	  print_files(cli->uploaders[i].peer_files);
   	  cout<<"--------------------------------------------------------------------------------------\n";
   }
}

