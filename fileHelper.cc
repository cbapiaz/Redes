#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

#include <sstream>
#include <string>

#include "fileHelper.hh"

using namespace std;



//unsigned char result[MD5_DIGEST_LENGTH];

// Print the MD5 sum as hex-digits.
void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

//return md5 string from a file
string getMD5(string file, int &fd,unsigned long &size) {
    unsigned char result[MD5_DIGEST_LENGTH];
    int file_descript;


    unsigned long file_size;
    char* file_buffer;            
    
    file_descript = open(file.c_str(), O_RDONLY);
    
    if(file_descript < 0)  {
        perror("\nCannot open file, invalid file descriptor\n");
        return "";
    }
    else {

        file_size = get_size_by_fd(file_descript);
    //    printf("file size:\t%lu\n", file_size);

        file_buffer = (char *)mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
        MD5((unsigned char*) file_buffer, file_size, result);

        stringstream st;
        st << result;    
        
        fd = file_descript;
        size = file_size;
      //  printf("MD5 for %s is:",file.c_str());        
      //  print_md5_sum((unsigned char*)st.str().c_str());
        printf("\n");

        return st.str();
    }
    /*print_md5_sum(result);
    printf("  %s\n", argv[1]);

    return 0;*/
}