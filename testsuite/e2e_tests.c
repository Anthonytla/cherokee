#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <criterion/criterion.h>
#include <ctype.h>
#include "../include/response.h"

#define BUFFER_SIZE 1024

void parse_response_status_line(char *status_line, struct Response *response) {
    if (!status_line) {return;}
    char *status_line_copy = strdup(status_line);
    char *http_version = strtok(status_line_copy, " ");
    char *status_code = strtok(NULL, " ");
    char *reason_phrase = strtok(NULL, "");
    if (!http_version || !status_code || !reason_phrase) {return;}
    response->status_line.http_version = strdup(http_version);
    response->status_line.status_code = atoi(status_code);
    reason_phrase[strlen(reason_phrase) - 1] = '\0';
    response->status_line.reason_phrase = strdup(reason_phrase);
    free(status_line_copy);
}

struct Response parse_response(char response_raw[BUFFER_SIZE]) {
    printf("response raw : \n%s\n", response_raw);
    struct Response response;
    char *res_copy;
    res_copy = strdup(response_raw);
    // printf("response copy : \n%s\n", res_copy);
    char *status_line = strtok(res_copy, "\n");
    printf("Status Line: %s\n", status_line);
    if (!status_line) {
        fprintf(stderr, "Invalid response\n");
        exit(EXIT_FAILURE);
    }

    parse_response_status_line(status_line, &response);

    char *body = strstr(response_raw, "\r\n\r\n");
    printf("Body: %s\n", body);
    if (body) {
        body += 4;
        printf("Response Body:\n%s\n", body);
        response.message_body = strdup(body);
    }

    free(res_copy);
    return response;
}

void send_post_with_file(char *method, char *file_path, char *api_path, int sockfd, char *content_type) {
    // Step 4: Formulate the HTTP request with the file
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE,
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Length: %ld\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             method, api_path, "0.0.0.0", file_size, content_type);

    puts(request);
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("send");
        fclose(file);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            perror("send");
            fclose(file);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
}

struct Response my_curl(char *http_method, char *api_path, char *file_path, char *content_type) {
    const char *api_host = "0.0.0.0";
    const char *api_port = "3490";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(api_host, api_port, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect");
        freeaddrinfo(result);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    if (strcmp(http_method, "POST") == 0 || strcmp(http_method, "PUT") == 0) {
        send_post_with_file(http_method, file_path, api_path, sockfd, content_type);
    } else {
        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "%s %s HTTP/1.1\r\nHost: %s\r\n\r\n", http_method, api_path, api_host);

        if (send(sockfd, request, strlen(request), 0) == -1) {
            perror("send");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    char response_raw[BUFFER_SIZE];
    ssize_t bytes_received;
    size_t response_size = 0;
    // struct Response response;
    while ((bytes_received = recv(sockfd, response_raw + response_size, sizeof(response_raw) - response_size - 1, 0)) > 0) {
        // response_raw[bytes_received] = '\0';
        // printf("%s\n", response_raw);
        // response = parse_response(response_raw);
        response_size += bytes_received;
    }
    if (bytes_received == -1) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    response_raw[response_size] = '\0';
    
    char *complete_response = (char *)malloc(response_size + 1);  // +1 for the null-terminator
    if (!complete_response) {
        perror("malloc");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    strcpy(complete_response, response_raw);
    struct Response parsed_response = parse_response(complete_response);
    free(complete_response);
    close(sockfd);
    return parsed_response;
}

TestSuite(get_method);
Test(get_method, should_return_200) {
    struct Response res = my_curl("GET", "/files/test.txt", NULL, NULL);
    cr_assert(res.status_line.status_code == 200, "Status code should be 200");
    cr_assert_str_eq(res.status_line.reason_phrase, "OK");
    cr_assert_str_eq(res.message_body, "Hello world");
}

Test(get_method, should_return_404) {
    struct Response res = my_curl("GET", "/files/does_not_exist.txt", NULL, NULL);
    cr_assert(res.status_line.status_code == 404, "Status code should be 404");
    cr_assert_str_eq(res.status_line.reason_phrase, "ERROR");
}

char* postRelativePath = "/files/files_to_upload/test.txt";
void post_setup(void) {
    remove("files/new_test.txt");
    if (access(postRelativePath, F_OK) == -1) {
        fopen(postRelativePath, "w");
    }
}

void post_cleanup(void) {
    remove("files/new_test.txt");
}

TestSuite(post_method);
Test(post_method, should_return_201) {
    // remove("files/new_test.txt");
    if (access(postRelativePath, F_OK) == -1) {
        fopen(postRelativePath, "w");
    }
    struct Response res = my_curl("POST", "/files/new_test", "files/files_to_upload/test.txt", "text/plain");
    cr_assert(res.status_line.status_code == 201, "Status code should be 201");
    cr_assert_str_eq(res.status_line.reason_phrase, "Created");
    remove("files/new_test.txt");
}

Test(post_method, should_return_409_if_file_already_exists) {
    // remove("files/new_test.txt");
    if (access(postRelativePath, F_OK) == -1) {
        fopen(postRelativePath, "w");
    }
    struct Response res = my_curl("POST", "/files/files_to_upload/test", "files/files_to_upload/test.txt", "text/plain");
    cr_assert(res.status_line.status_code == 409, "Status code should be 409");
    cr_assert_str_eq(res.status_line.reason_phrase, "File already exists");
}

char* deleteRelativePath = "files/test-file-to-delete.txt";
void delete_setup(void) {
    if (access(deleteRelativePath, F_OK) == -1) {
        fopen(deleteRelativePath, "w");
    }
}

TestSuite(delete_method, .init = delete_setup);
Test(delete_method, should_return_200) {
    struct Response res = my_curl("DELETE", "/files/test-file-to-delete.txt", NULL, NULL);
    cr_assert(res.status_line.status_code == 200, "Status code should be 200");
    cr_assert_str_eq(res.status_line.reason_phrase, "OK");
}

Test(delete_method, should_return_404_if_file_does_not_exist) {
    struct Response res = my_curl("DELETE", "/files/asdczdjsjcziceifci.txt", NULL, NULL);
    cr_assert(res.status_line.status_code == 404, "Status code should be 200");
    cr_assert_str_eq(res.status_line.reason_phrase, "ERROR");
}

char* putRelativePath = "files/files_to_upload/test_put.txt";
void put_setup(void) {
    remove("files/new_test_put.txt");
    if (access(putRelativePath, F_OK) == -1) {
        fopen(putRelativePath, "w");
    }
}

void put_cleanup(void) {
    remove("files/new_test_put.txt");
}

TestSuite(put_method, .init = put_setup, .fini = put_cleanup);
Test(put_method, should_return_201) {
    struct Response res = my_curl("PUT", "/files/new_test_put", "files/files_to_upload/test_put.txt", "text/plain");
    cr_assert(res.status_line.status_code == 201, "Status code should be 201");
    cr_assert_str_eq(res.status_line.reason_phrase, "Created");
}

Test(put_method, should_return_200_if_file_already_exists) {
    struct Response res = my_curl("PUT", "/files/files_to_upload/test_put", "files/files_to_upload/test_put.txt", "text/plain");
    printf("status code : %d", res.status_line.status_code);
    cr_assert(res.status_line.status_code == 200, "Status code should be 200");
    cr_assert_str_eq(res.status_line.reason_phrase, "OK");
}