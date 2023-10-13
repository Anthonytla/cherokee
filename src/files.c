#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/files.h"
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void read_dir(DIR *dir, char *buffer) {
    struct dirent *entry = NULL;
   while ((entry = readdir(dir)) != NULL) {
        strcat(buffer, entry->d_name);
        strcat(buffer, "\n");
    }
}

int checkFileExist(char *filename) {
    return (access(filename, F_OK) != -1);
}


int checkFolderExist(char *foldername) {
  return open(foldername, O_RDONLY);
}

int fileExists(const char *path) {
    struct stat fileStat;
    
    if (stat(path, &fileStat) == 0) {
        return S_ISREG(fileStat.st_mode);
    }
    
    // Handle the error if stat() fails
    return -errno;
}

int isDirectory(const char* filePath) {
    struct stat fileStat;
    
    if (stat(filePath, &fileStat) == 0) {
        return S_ISDIR(fileStat.st_mode);
    }
    
    // Handle the error if stat() fails
    return -errno;
}
