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
  	int fd;///////
  	unsigned long size;
  	unsigned long bytes_transfered;
} fileDescriptor;


// A peer is another client connected to me by and upload or a download
typedef struct peer {
	string ip;
	string port;
	fileDescriptor peer_file; //file being downloaded or uploaded
} peer;

//define the client structure
typedef struct client
{
        	string ip;
			string port;
			map<int,peer> uploads; //map by socket, store information about the files i am uploading
			map<int,peer> downloads; //map by socket, store information about the files i am downloading
			map<string,fileDescriptor> shared_files; //map by filename, file that i share with other clients or peers
} client;

//define the trackerClient structure
typedef struct trackerClient
{
        	string ip;
			string port;
			map<string,fileDescriptor> client_files; //map by filename
} trackerClient;


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

string getFileMD5(client * cli, string file){	
	return cli->shared_files[file].md5;
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

			 cli->shared_files[file] = fdesc;

		 }
		 else {
		 	perror("\nCannot share file, error calculating md5\n");
		 }
	}
	else perror ("share_file: client should not be null");
}

void print_file(fileDescriptor& f) {	
   	  cout<<"File: '"<<f.name << "' - MD5: "; print_md5_sum((unsigned char*)f.md5.c_str());cout<<" - Bytes: " <<f.bytes_transfered << "\n";
}


void print_files(map<string,fileDescriptor>& m) {	
   for(std::map<string,fileDescriptor>::const_iterator it = m.begin(); it != m.end(); it++)
   {  fileDescriptor _fd = (fileDescriptor)(it->second);
   	  print_file(_fd);
   }
}


void print_shared_files(client * cli){
	cout<<"\nShared files\n";
	print_files(cli->shared_files);	  
}

//Muestra las descargas en progreso junto con la dirección del uploader y los bytes descargados.
void print_downloads(client * cli){
   cout<<"\nDownloading files\n";   
   for(std::map<int, peer>::const_iterator it = cli->downloads.begin(); it != cli->downloads.end(); it++)
   {
   	 peer p = it->second;
   	 cout<<"Downlading From: "<< p.ip << "@" << p.port <<"\n";  	  
   	 cout<<"--------------------------------------------------------------------------------------\n";
   	 print_file(p.peer_file);
   	 cout<<"--------------------------------------------------------------------------------------\n";
   }
}

//Muestra las cargas en progreso junto con la dirección del downloader y los bytes entregados.
void print_uploads(client * cli){
   cout<<"\nUploadding files\n";   
   for(std::map<int, peer>::const_iterator it = cli->uploads.begin(); it != cli->uploads.end(); it++)
   {
   	  peer p = it->second;
   	  cout<<"Uploading to: "<< p.ip << "@" << p.port <<"\n";
   	  cout<<"--------------------------------------------------------------------------------------\n";
 	  print_file(p.peer_file);
 	  cout<<"--------------------------------------------------------------------------------------\n"; 	     	  
   }

}

/***************TRACKER METHODS******************************/
void addNewTrackerClient(map<int,trackerClient*> &trackerClients,int fd, string ip, string port){
	trackerClients[fd] = new trackerClient();
	trackerClients[fd]->ip = ip;
	trackerClients[fd]->port = port;
}

string get_md5_string(unsigned char* md) {
    int i;
    string result="";
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
    		char buff[255];
            sprintf(buff,"%02x",md[i]);
            string s(buff);
            result=result+s;
    }
    return result;
}

string search_file(map<int,trackerClient*> &clients,string file) {
	string md5="";
	string clientsWithTheFile="";

	map<int, trackerClient*>::const_iterator it= clients.begin();
	bool foundFirst = false;	
	while (!foundFirst && it != clients.end())
    {
    	map<string, fileDescriptor>::const_iterator itFiles=it->second->client_files.begin();    	
    	while(!foundFirst && itFiles != it->second->client_files.end())
    	{
    		foundFirst = itFiles->first.compare(file) == 0;
    		
    		if (!foundFirst)
    			itFiles++;
    	}
    	
    	if (!foundFirst)
    		it++;
    	else md5 = itFiles->second.md5;
    }     	 

 	if (!foundFirst) {
 		return "fail\nFile not found";
 	}
 	else {
 		bool foundAny = false;
 		for(std::map<int, trackerClient*>::const_iterator it = clients.begin(); it != clients.end(); it++)
    	{
    		bool found = false;
    		for(std::map<string, fileDescriptor>::const_iterator itFiles = it->second->client_files.begin(); itFiles != it->second->client_files.end(); itFiles++)
    		{
    			found = (itFiles->second.md5 == md5);
    			if (found) break;
    		}

    		if (found) {
    			clientsWithTheFile += it->second->ip + ":" + it->second->port+"\n";
    		}

    		foundAny = foundAny || found;
    	}

    	if (foundAny) {
    	  return "FILE\n"+get_md5_string((unsigned char*)md5.c_str())+"\n"+clientsWithTheFile+"\n";
    	}
    	else return "fail\ncritical error MD5 not found";
 	}

    return "";
}


string publish_file(trackerClient *cli,string file,string _md5) {
	if (cli != NULL) {		 		 
		
		if (cli->client_files.find(file) == cli->client_files.end()) {
		     fileDescriptor fdesc;
		     fdesc.name = file;			
			 fdesc.md5 = _md5;
			 fdesc.fd = -1;
			 fdesc.size = -1;
			 fdesc.bytes_transfered = 0;
			 cli->client_files[file]=fdesc;//.push_back(
		}
		else return "Cannot publish the same file twice";
	}
	else return "Client should not be null";

	return "";
}




/******CLIENT TO CLIENT METHODS*******/

/*add new upload, if the peer upload does not exists we create a new one*/
void addUpload(client *cli, string ip, string port,int fd, string filename) {

}
/*add new download, if the peer download does not exists we create a new one*/
void addDownload(client *cli, string ip, string port,int fd, string filename) {

}

/*update the bytes of an existing upload by the fd socket*/
/* this also updates the shared file "filename" bytes*/
void updateUpload(client *cli, int fd_socket,int bytes, string filename) {

}

/*update the bytes of an existing download by the fd socket*/
void updateDownload(client *cli, int fd_socket,int bytes, string filename) {

}

void deleteUpload(client *cli, int fd_socket) {

}

void deleteDownload(client *cli, int fd_socket) {

}

/****************************************/



