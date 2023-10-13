#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include "../include/process.h"
#include "../include/request.h"
#include "../include/header.h"
#include "../include/request_handler.h"
#include "../include/server.h"
#include "../include/cache.h"
#include "../include/files.h"
#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#define SIZE 3000
#define PROCESSES 3
#define THREAD_POOL_SIZE 10
static pthread_mutex_t mutex;
pthread_t request_parsing_pool[THREAD_POOL_SIZE];
pthread_cond_t sig;
struct queue *queue = NULL;
int max_socket;
struct Task tasks[8000];

void sigpipe_handler(int signal) {
    if (signal == 13) {
        // for (int i = 0; i < max_socket; i++) {
        //     close(tasks[i].client_socket);
        //     tasks[i].client_socket = -1;
        //     close(tasks[i].fileFd);
        //     tasks[i].fileFd = -1;
        // }
    }
    // Handle the SIGPIPE signal here, e.g., close the socket gracefully.
    // For demonstration purposes, we'll just print a message.
    // printf("Received SIGPIPE signal\n");
}
void sigint_handler(int signal) {
    printf("sigint %d\n", signal);
    pthread_mutex_lock(&mutex);
    queue_destroy(queue);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&sig);
    int v;
    check(v = kill(0, SIGKILL), "kill failed");
    printf("%p\n", (void *) queue);

}
// Génère les processus selon le fichier de configuration grâce à la méthode fork() 
// puis les fait surveiller le server_socket
void spawn_process(char *port) {
    
    int server_socket = init_server(port);
    for (int i = 0; i < PROCESSES; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork error");
        }
        else if (pid == 0) {
            printf("Created process pid %d/pgid %d\n", getpid(), getppid());
            queue = queue_init();
            for (int i = 0; i < THREAD_POOL_SIZE; i++) {
                pthread_create(&request_parsing_pool[i], NULL, requestParsingThread, NULL);
            }
            watch(server_socket);
            puts("end loop");
            queue_destroy(queue);
        }
    }
}

void waitForAll() {
	int status;
	pid_t pid;
	int n = 0;
	while (n < PROCESSES) {
		pid = wait(&status);
        
        if (pid == -1) {
            perror("wait");
            // Handle wait error if necessary
            continue;
        }
        
        if (WIFEXITED(status)) {
            printf("Fork [%i] terminated with exit status: %i\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Fork [%i] terminated by signal: %i\n", pid, WTERMSIG(status));
        }
        n++;
    }
}

int isSocket(int fd) {
    // printf("fd-%d\n",fd);
    struct stat fdStat;
    if (fstat(fd, &fdStat) == -1) {
        perror("fstat 2 failed");
        return -1;
    }
    return S_ISSOCK(fdStat.st_mode);
}

/**
 * Watch for new connections and handle them
 * 
*/
void watch(int server_socket) {
    fd_set readfds;
    fd_set writefds;

    fd_set master_write;
    fd_set master_read;
    max_socket = server_socket;
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&sig, NULL);
    FD_ZERO(&master_read);
    FD_ZERO(&master_write);
    FD_SET(server_socket,&master_read);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    //printf("socket %d\n", max_socket);
    // le select réagit quand un evenement a lieu sur une socket
    for (;;) {
         signal(SIGINT, sigint_handler);
        //printf("%d\n", getpid());
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        readfds = master_read;
        writefds = master_write;
        int rv = select(max_socket + 1, &readfds, &writefds, NULL,&timeout);
        if (rv < 0) {
            perror("select");
        }
        if (FD_ISSET(server_socket, &readfds)) {
           
                int new_sd;
                new_sd = accept(server_socket, NULL, NULL);
                
                FD_SET(new_sd, &master_read);
                if (new_sd > max_socket) {max_socket = new_sd;}
        }
        // création du task qui va être passé en argument aux threads de parsing 
        // et de réponse pour qu'ils puissent communiquer entre eux et avec le main thread 
        // On parcourt les file descriptors pour voir sur lequel il y a eu un evenement
        for (int i = server_socket + 1; i <= max_socket; i++) {
            if (FD_ISSET(i, &readfds)) {
                if (isSocket(i)) {
                    if (max_socket < i) {
                        max_socket = i;
                    }
                    char request[SIZE];
                    bzero(request, SIZE);
                    int bytesRead;
                    tasks[i].request = calloc(1,SIZE);
                    tasks[i].sharedfd.writefds = &master_write;
                    tasks[i].sharedfd.readfds = &master_read;
                    unsigned long totalBytesRead = 0;
                    unsigned long contentLength = 0;
                    char *messageBody = NULL;
                    size_t header_size = 0;
                    while ((bytesRead = recv(i, request, SIZE, MSG_DONTWAIT))> 0) {

                        if((messageBody = strstr(request, "\n\n")) != NULL) {
                            size_t request_size = strlen(request);
                            header_size = request_size - strlen(messageBody);
                            contentLength = get_message_body_length(request);
                        }
                        
                        if(totalBytesRead + bytesRead > SIZE) {
                            tasks[i].request = realloc(tasks[i].request, totalBytesRead + bytesRead + 1);
                        }
                        memcpy(&tasks[i].request[totalBytesRead], request, bytesRead);
                        totalBytesRead += bytesRead;
                        bzero(request, bytesRead);
                        
                        if((header_size  > 0) && (totalBytesRead - header_size >= contentLength)) {
                            break;
                        }
                        
                    }
                    tasks[i].request[totalBytesRead] = '\0';
                    if (!(tasks[i].request && tasks[i].request[0] != '\0')){
                        FD_CLR(tasks[i].client_socket, tasks[i].sharedfd.readfds);
                        close(tasks[i].client_socket);
                        tasks[i].client_socket = -1;
                        break;
                    }
                   
                    int *pi = malloc(sizeof(int));
                    *pi = i;
                    tasks[i].client_socket = *pi;
                    pthread_mutex_lock(&mutex);
                    queue_push(queue, pi);
                    
                    pthread_mutex_unlock(&mutex);
                }
                else {
                    int *fd = malloc(sizeof(int));
                    *fd = i;
                    pthread_t thread2;
                    pthread_create(&thread2, NULL, responseBuildThread, fd);
                    pthread_join(thread2, NULL);
                    struct Task task2 = tasks[i];
                    
                    FD_CLR(task2.fileFd, task2.sharedfd.readfds);
                    FD_CLR(task2.client_socket, task2.sharedfd.readfds);
                    close(task2.client_socket);
                    close(task2.fileFd); 
                }
            }

            if (FD_ISSET(i, &writefds)) {
                pthread_t thread3;
                pthread_create(&thread3, NULL, responseBuildThreadPut, &i);
                pthread_join(thread3, NULL);
            }

        }
    }

}    

/**
 * On récupère le client socket dans le task et on parse la request dans le request_struct
 * 
*/
void* requestParsingThread() {
    
    for (;;) {
        
        pthread_mutex_lock(&mutex);
        
        int *psocket = queue_pop(queue);
        pthread_mutex_unlock(&mutex);
        // printf("psocket %p\n", (void*)psocket);
        if (!psocket) {continue;}

        int socket = *psocket;
        struct Task task2 = tasks[socket];
  
        parse_request(task2.request, &task2.request_struct);
        int fd = 0;
        if (is_get_method(task2.request_struct)){
                fd = get(task2.request_struct, &task2.response );
                // si le fd est positif (pas une erreur), on l'enregistre dans le task et on l'ajoute au readfds
                if (fd > 0){
                    task2.fileFd = fd;
                    tasks[fd] = task2;
                    FD_SET(task2.fileFd, task2.sharedfd.readfds);
                }
                else {
                    generateResponseHeader(task2.request_struct.request_line.request_uri, errno_to_status(-fd), "ERROR", &task2.response);
                }
        } else if( is_post_method(task2.request_struct)) {
            fd = post(task2.request_struct, &task2.response );
            printf("post file descriptor %d", fd);
            if (fd > 0){
                task2.fileFd = fd;
                tasks[fd] = task2;
                FD_SET(task2.fileFd, task2.sharedfd.writefds);
            }
        } else if (is_delete_method(task2.request_struct)) {
            fd = delete(task2.request_struct, &task2.response );
            printf("delete %d\n", fd);
            if (fd > 0) {
                generateResponseHeader(task2.request_struct.request_line.request_uri, 200, "OK", &task2.response);
            }
            else {
                generateResponseHeader(task2.request_struct.request_line.request_uri, errno_to_status(-fd), "ERROR", &task2.response);
            }
            // char headers[555];
            // sprintf(headers, "%s %d %s\r\nContent-Type: %s\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);
            // check(send(task2.client_socket, headers, strlen(headers), 0), "send header failed");
            // si methode PUT 
        } else if (is_put_method(task2.request_struct)) {
            fd = put(task2.request_struct, &task2.response);
            // printf("put %d\n", fd);
            // si le fd est positif (pas une erreur), on l'enregistre dans le task et on l'ajoute au writefds
            if (fd > 0){
                task2.fileFd = fd;
                tasks[fd] = task2;
                FD_SET(task2.fileFd, task2.sharedfd.writefds);
            }
        } else if (is_custom_method(task2.request_struct)) {
            custom(task2.request_struct, &task2.response);
            char headers[255];
            sprintf(headers, "%s %d %s\nContent-Type: %s\n\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);
            check(send(task2.client_socket, headers, strlen(headers), 0), "send headers failed");
            check(send(task2.client_socket, task2.response.message_body, strlen(task2.response.message_body), 0), "send failed");
            close(task2.client_socket);
            FD_CLR(task2.client_socket, task2.sharedfd.readfds);
            printf("custom method : done\n");
        } else if (is_head_method(task2.request_struct)) {
            head(task2.request_struct, &task2.response);
            time_t now;
            struct tm* gmt;
            char date[100];
            now = time(0);
            gmt = gmtime(&now);
            strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);
            char headers[255];
            sprintf(headers, "%s %d %s\r\nDate: %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\nServer: Custom Server\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, date, task2.response.header, task2.response.content_length);
            check(send(task2.client_socket, headers, strlen(headers), 0), "send headers failed");
            check(send(task2.client_socket, task2.response.message_body, strlen(task2.response.message_body), 0), "send failed");
            printf("head method : done\n");
        } 
        else { // bad method
            char headers[555];
            generateResponseHeader(task2.request_struct.request_line.request_uri, 402, "Bad Request", &task2.response);
            sprintf(headers, "%s %d %s\r\nContent-Type: %s\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);
            check(send(task2.client_socket, headers, strlen(headers), 0), "send header failed");
            close(task2.client_socket);
            FD_CLR(task2.client_socket, task2.sharedfd.readfds);
            printf("request error : bad method");
        } 

        if (fd < 0 || is_delete_method(task2.request_struct)) {
            char headers[555];
            sprintf(headers, "%s %d %s\r\nContent-Type: %s\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);
            puts(headers);
            check(send(task2.client_socket, headers, strlen(headers), 0), "send header failed");
            close(task2.client_socket);
            FD_CLR(task2.client_socket, task2.sharedfd.readfds);
        }
        if (max_socket < fd) {
            max_socket = fd;
        }
        
    }
}

/**
 * On construit la réponse suite à une requête GET
*/
void* responseBuildThread(void *arg) {

            int *pfd = (int *)arg;
            int fd = *pfd;
            free(pfd);
            struct Task task2 = tasks[fd];
            
            char headers[255];
            sprintf(headers, "%s %d %s\r\nContent-Type: %s\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);
            
            check(send(task2.client_socket, headers, strlen(headers), 0), "send headers failed");

            DIR *dir = fdopendir(task2.fileFd);
            if (dir) {
                task2.response.message_body = calloc(1, SIZE);
                struct dirent *entry;
                size_t size = 0;
                while ((entry = readdir(dir)) != NULL) {
                // Process the directory entry
                    size += strlen(entry->d_name);
                    if (size > SIZE){
                        task2.response.message_body = realloc(task2.response.message_body, size+1);
                    }
                    
                    strcat(task2.response.message_body, entry->d_name);    
                    strcat(task2.response.message_body, "\n");
                    
                }
                check(send(task2.client_socket, task2.response.message_body, strlen(task2.response.message_body), 0), "send failed");
                free(task2.response.message_body);
            } 
            else {
                off_t offset = 0;
                struct stat fileStat;
                fstat(task2.fileFd, &fileStat);
                off_t fileSize = fileStat.st_size;
                int bytesSent;
                
                signal(SIGPIPE, sigpipe_handler);
                check(bytesSent = sendfile(task2.client_socket, task2.fileFd, &offset, fileSize), "send files failed");
                
            }
            pthread_exit(NULL);
}

/**
 * On construit la réponse suite à une requête PUT
*/
void* responseBuildThreadPut(void* arg) {
   int *pfd = (int *)arg;
    int fd = *pfd;
    struct Task task2 = tasks[fd];

    char response[SIZE];
    bzero(response, SIZE);
    char headers[255];
    sprintf(headers, "%s %d %s\r\nContent-Type: %s\r\n\r\n", task2.response.status_line.http_version, task2.response.status_line.status_code, task2.response.status_line.reason_phrase, task2.response.header);

    check(send(task2.client_socket, headers, strlen(headers), 0), "send headers failed"); 
    check(write(task2.fileFd, task2.request_struct.message_body, task2.request_struct.body_length), "write failed");
    
    close(task2.fileFd);
    FD_CLR(task2.fileFd, task2.sharedfd.writefds);
    close(task2.client_socket);
    FD_CLR(task2.client_socket, task2.sharedfd.readfds);
    pthread_exit(NULL);
}
