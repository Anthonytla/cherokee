#include "../include/request.h"
#include "../include/response.h"
#include "../include/utils.h"
#include "../include/header.h"
#include "../include/request_handler.h"
#include "../include/files.h"
#include "../include/cache.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <dlfcn.h>



char *trim_whitespace(const char *str) {
    // Find the first non-whitespace character
    const char *start = str;
    while (isspace(*start)) {
        start++;
    }

    // Find the last non-whitespace character
    const char *end = str + strlen(str) - 1;
    while (end > start && isspace(*end)) {
        end--;
    }

    // Calculate the length of the trimmed string
    size_t trimmed_len = end - start + 1;

    // Create a copy of the trimmed string
    char *trimmed_str = malloc((trimmed_len + 1) * sizeof(char));
    if (trimmed_str == NULL) {
        return NULL; // Memory allocation failed
    }
    strncpy(trimmed_str, start, trimmed_len);
    trimmed_str[trimmed_len] = '\0';

    return trimmed_str;
}

char *extract_extension(const char *media_type) {
    // Remove leading and trailing whitespace
    char *trimmed_type = trim_whitespace(media_type);
    if (trimmed_type == NULL) {
        return NULL; // Memory allocation failed
    }

    // Find the last '/' character
    const char *last_slash = strrchr(trimmed_type, '/');
    printf("last slash:%s\n", last_slash);
    if (last_slash == NULL || *(last_slash + 1) == '\0') {
        free(trimmed_type);
        return NULL; // Invalid media type format
    }

    // Extract the extension
    const char *extension_start = last_slash + 1;
    printf("extension start:%s\n", extension_start);
    // Map media types to extensions
    const char *extension = NULL;
    if (strcasecmp(extension_start, "plain") == 0) {
        extension = "txt";
    } else if (strcasecmp(extension_start, "html") == 0) {
        extension = "html";
    } else if (strcasecmp(extension_start, "css") == 0) {
        extension = "css";
    } else if (strcasecmp(extension_start, "javascript") == 0) {
        extension = "js";
    } else if (strcasecmp(extension_start, "jpeg") == 0) {
        extension = "jpg";
    } else if (strcasecmp(extension_start, "png") == 0) {
        extension = "png";
    } else if (strcasecmp(extension_start, "json") == 0) {
        extension = "json";
    } else if (strcasecmp(extension_start, "pdf") == 0) {
        extension = "pdf";
    } else if (strcasecmp(extension_start, "xml") == 0) {
        extension = "xml";
    } else {
        free(trimmed_type);
        return NULL; // Unsupported media type
    }
    printf("extension:%s\n", extension);
    char *extension_copy = malloc((strlen(extension) + 1) * sizeof(char));
    if (extension_copy == NULL) {
        free(trimmed_type);
        return NULL; // Memory allocation failed
    }
    strcpy(extension_copy, extension);

    free(trimmed_type);
    return extension_copy;
}

/**
 * Create or update the file with the given path and extension, depending if 
 * it's a POST or PUT request
*/
int createFileWithBody(const char* filePath, const char* extension, int forPost, struct Response *response) {
    // Remove the first slash if it exists
    const char* cleanFilePath = (filePath[0] == '/') ? filePath + 1 : filePath;

    // Extract the directory path and file name
    char directory[256];
    char fileName[256];
    char* lastSlash = strrchr(cleanFilePath, '/');
    if (lastSlash != NULL) {
        strncpy(directory, cleanFilePath, lastSlash - cleanFilePath + 1);
        directory[lastSlash - cleanFilePath + 1] = '\0';
        strcpy(fileName, lastSlash);
    } else {
        strcpy(directory, "");
        strcpy(fileName, cleanFilePath);
    }

    // Verify that the first folder is "files"
    char* firstSlash = strchr(directory, '/');
    if (firstSlash == NULL || (firstSlash - directory != 5) || strncmp(directory, "files", 5) != 0) {
        printf("Invalid directory path\n");
        return -1;
    }

    // Create the directory for the file if it doesn't exist
    int dirResult = mkdir(directory, 0700);
    if (dirResult != 0 && errno != EEXIST) {
        perror("Error creating the directory");
        return -1;
    }

    // Create the full file path
    char fullFilePath[1024];
    snprintf(fullFilePath, sizeof(fullFilePath), "%s/%s.%s", directory, fileName, extension);
    printf("Full File Path: %s\n", fullFilePath);

    // Check if the file already exists
    struct stat st;
    if (forPost ){
      if (stat(fullFilePath, &st) == 0) {
        // File exists, return an appropriate response (e.g., error code)
        generateResponseHeader(filePath, 409, "File already exists", response);
        return -1;
      }
      else {
        generateResponseHeader(filePath, 201, "Created", response);
      }
    }
    else {
      if (stat(fullFilePath, &st) == 0) {
        // File exists, return an appropriate response (e.g., error code)
        generateResponseHeader(fullFilePath, 200, "OK", response);
      }
      else {
        generateResponseHeader(fullFilePath, 201, "Created", response);
      }
    }

    // Reopen the file to get the file descriptor
    int fd = open(fullFilePath, O_CREAT| O_TRUNC | O_RDWR, 0777);
    
    if (fd == -1) {
        perror("Error opening the file");
        generateResponseHeader(filePath, errno_to_status(errno), "Error opening the file", response);
        return -1;
    }

    // Return the file descriptor
    return fd;
}

/**
 * Gère les requêtes GET et renvoie le fd du fichier ou dossier demandé
*/
int get(struct Request request, struct Response *response)
{
  
  char *request_uri = request.request_line.request_uri;
  char *filepath = request_uri + 1;

  // si pas de request_uri on renvoie une erreur
  if (!filepath)
  {
    // set_response_status_line(404, "url not found", response);
    generateResponseHeader(request_uri, 404, "url not found", response);
    return -2;
  }

  int fd;
  // si c'est un dossier on ouvre le dossier et on récupère le fd pour le renvoyer
  if (isDirectory(filepath) > 0) {
    DIR *dir = opendir(filepath);
    if (!dir) {
        generateResponseHeader(filepath, errno_to_status(errno), "Directory not found", response);
      return -errno;
    }
    fd = dirfd(dir);
  }
  // sinon on ouvre le fichier et on récupère le fd pour le renvoyer
  else {
    fd = open(filepath, O_RDONLY, 777); 
    if (fd < 0) {
        generateResponseHeader(filepath, errno_to_status(errno), "Error", response);
        return -errno;
    }
  }
  generateResponseHeader(filepath, 200, "OK",response);

  return fd;

}

void head(struct Request request, struct Response *response)
{
  char *request_uri = request.request_line.request_uri;
  char *filepath = request_uri + 1;
  response->message_body = "";
  response->content_length = 0;

  // si pas de request_uri on renvoie une erreur
  if (!filepath)
  {
    generateResponseHeader(request_uri, 404, "url not found", response);
  }

  // Check if the path starts with "files/"
  if (strncmp(filepath, "files/", 6) != 0) {
      generateResponseHeader(request_uri, 404, "bad request", response);
  }

    // Open the file to check if it exists
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        generateResponseHeader(request_uri, 404, "resource not found", response);
    }

  // File exists and can be opened, so we can get the content length
  fseek(file, 0, SEEK_END);
  response->content_length = (size_t)ftell(file);
  fclose(file);

  generateResponseHeader(request_uri, 200, "OK",response);
}


/**
 * Gère les requêtes PUT et renvoie le fd du fichier demandé, 
 * en le créant si besoin et en écrivant le message body dedans 
*/
int put(struct Request request, struct Response *response)
{

  char *request_uri = request.request_line.request_uri;
  char *filepath = request_uri + 1;
  // si pas de request_uri on renvoie une erreur
  if (!request_uri)
  {
    generateResponseHeader(request_uri, 404, "Not found", response);
  }
   int fd;

  struct ContentTypeHeader *content_type_header = request.header.content_type_header;
  char *media_type = NULL;
  if (content_type_header != NULL) {
    media_type = content_type_header->media_type;
    printf("media type:%s\n", media_type);
  }
  
  fd = createFileWithBody(filepath, extract_extension(media_type), 0, response);
  if (fd < 0) {
    generateResponseHeader(filepath , errno_to_status(errno), "ERROR", response);
  }

  // on renvoie le fd du fichier 
  return fd;

}

/**
 * Gère les requêtes DELETE et renvoie un code d'erreur ou de réussite
*/
int delete(struct Request request, struct Response *response) {

  char *request_uri = request.request_line.request_uri;
  if (!request_uri)
  {
    generateResponseHeader(request_uri, 404, "Not found", response);
    return -2;
  }

  if (check(remove(request_uri+1), "delete failed")) {
    printf("deleted !");
    generateResponseHeader(request_uri, 200, "OK", response);
    //set_response_status_line(200, "file deleted successfully", response);
    return 1;
  }
  //set_response_status_line(500, "Something went wrong...", response);
  generateResponseHeader(request_uri, errno_to_status(errno), "Delete failed", response);
  return -errno; 
}

/**
 * Gère les requêtes POST
*/
int post(struct Request request, struct Response *response) {

  struct ContentTypeHeader *content_type_header = request.header.content_type_header;
  char *media_type = NULL;
    if (content_type_header != NULL) {
      media_type = content_type_header->media_type;
      printf("media type:%s\n", media_type);
    }

    char *request_uri = request.request_line.request_uri;
    if (!request_uri)
    {
      printf("no request uri\n");
      generateResponseHeader(request_uri, 400, "Bad Request", response);
      return -1;
    }

    int file_fd = createFileWithBody(request_uri, extract_extension(media_type), 1,response);
    return file_fd;
}

/**
 * Gère les requêtes CUSTOM en faisant appel à la librairie dynamique et à la 
 * fonction spécifiées dans l'URI
*/
int custom(struct Request request, struct Response *response) {
  char *request_uri = request.request_line.request_uri;
  response->message_body = "";
  if (!request_uri)
  {
      printf("no request uri\n");
      generateResponseHeader(request_uri, 400, "Bad Request", response);
      return -1;
  }

  const char* func_path = (request_uri[0] == '/') ? request_uri + 1 : request_uri;

  // Extract the directory path and file name
  char lib_name[256];
  char func_name_slash[256];
  char* lastSlash = strrchr(func_path, '/');
  if (lastSlash != NULL) {
      strncpy(lib_name, func_path, lastSlash - func_path + 1);
      lib_name[lastSlash - func_path] = '\0';
      strncpy(func_name_slash, lastSlash, *lastSlash + 1);
  } else {
      printf("error : incorrect lib/func pattern\n");
      generateResponseHeader(request_uri, 400, "Bad Request", response);
      return -1;
  }

  char *func_name = &func_name_slash[1];

  printf("lib name: %s\n", lib_name);
  printf("func name: %s\n", func_name);
  
  // all libraries must be placed in the ./dynamic/ directory
  char librarypath[256] = "dynamic/";
  strcat(librarypath, lib_name);

  // get library and check if it exists
  void *libhandle = dlopen(librarypath, RTLD_LAZY);
  if (!libhandle) {
    fprintf(stderr, "dlopen error:\n%s\n", dlerror());
    generateResponseHeader(request_uri, 404, "Not found", response);
    return -1;
  }
  printf("library loaded\n");

  // get function and check if it exists
  char *(*opfunc) (char *);
  *(void **)(&opfunc) = dlsym(libhandle, func_name);
  if (!opfunc) {
    fprintf(stderr, "dlsym error:\n%s\n", dlerror());
    generateResponseHeader(request_uri, 404, "Not found", response);
    return -1;
  }
  printf("function loaded\n");

  char *result = opfunc(request.message_body);
  // printf("result:%s\n", result);
  response->message_body = strdup(result);
  free(result);

  generateResponseHeader(strcat(request.request_line.request_uri, ".txt"), 200, "OK", response);

  return 0;
}

