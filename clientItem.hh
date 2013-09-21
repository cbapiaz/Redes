#ifndef CLIENTITEM_H
#define CLIENTITEM_H

#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <map>

#include "fileHelper.hh"
using namespace std;

/*client structures*/
typedef struct fileDescriptor fileDescriptor;
typedef struct client client;
typedef struct peer peer;
//typedef struct conexion conexion;

/*tracker*/
typedef struct trackerClient trackerClient;


/** create a new client */
client * client_create (string ip, string port);

/** get client ip */
string client_getcIp (client *cli);

/** get client port */
string client_getcPort (client *cli);

/** destroy client */
void client_destroy (client *cli); 

void share_file(client *cli,string file);

void print_shared_files(client * cli);
void print_downloads(client * cli);
void print_uploads(client * cli);

string getFileMD5(client * cli, string file);


/******HANDLE CLIENT TO CLIENT*******/

/*add new upload, if the peer upload does not exists we create a new one*/
void addUpload(client *cli, string ip, string port,int fd_socket, string filename);
/*add new download, if the peer download does not exists we create a new one*/
void addDownload(client *cli, string ip, string port,int fd_socket, string filename);


/*update the bytes of an existing peer*/
/*invoqued by updateUpload and updateDownload*/
peer updateBytes(peer peerToUpdate, unsigned long bytes);

/*update the bytes of an existing upload by the fd socket*/
/* this also updates the shared file "filename" bytes*/
void updateUpload(client *cli, int fd_socket,unsigned long bytes, string filename);

/*update the bytes of an existing download by the fd socket*/
void updateDownload(client *cli, int fd_socket,unsigned long bytes, string filename);

void deleteUpload(client *cli, int fd_socket);
void deleteDownload(client *cli, int fd_socket);

/*get the filename wth a MD5*/
std::string getFilename (client *cli,std::string md5);

/****************************************/


/***************HANDLE TRACKER METHODS******************************/

void addNewTrackerClient(map<int,trackerClient*> &trackerClients,int fd,string ip, string port);

///tracker, checkear que no exista otro archivo con el mismo nombre
string publish_file(trackerClient *cli,string file,string _md5);

//recorro tracker clients, busco filename hasta encontrar uno, 
//con el md5 recorro y me quedo con los clientes asociados que tienen ese md5
string search_file(map<int,trackerClient*> &trackerClients,string file);

void remove_client(map<int,trackerClient*> &trackerClients, int fd);

/** get trackerclient ip */
string getcIp (trackerClient *cli);

/** get trackerclient port */
string getcPort (trackerClient *cli);

/*******************************************************************/

#endif 
