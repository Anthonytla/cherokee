#ifndef __FILES_H__
#define __FILES_H__
#include <sys/types.h>
#include <dirent.h>


void read_dir(DIR *dir, char *buffer);
int checkFileExist(char *filename);
int fileExists(const char *path);
int checkFolderExist(char *foldername);
int isDirectory(const char* filePath);
#endif
