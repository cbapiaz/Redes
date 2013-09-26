#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>
#include <openssl/md5.h>


using namespace std;

void print_md5_sum(unsigned char* md);
unsigned long get_size_by_fd(int fd);
string getMD5(string file, int &fd,unsigned long &size);

#endif

