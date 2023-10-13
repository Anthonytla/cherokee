#include <criterion/criterion.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/request_handler.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/cache.h"
#include "../include/header.h"

char **cache = NULL;
void setup(void)
{
    puts("setup");
}

void teardown(void)
{
    puts("teardown");
}
// TestSuite(http_method);
// Test(http_method, put, .init=setup, .fini=teardown) {
//     puts("test1");
//     remove("files/test.test");
//     char request[]="PUT http://cherokee/files/test.test 1.1\nContent-Type:text/json\n\njzoef";
//     struct Response *response = request_handling(request, cache);
//     cr_assert_eq(0,strcmp(response->message_body, "jzoef"));
//     printf("code 201 %d\n", response->status_line.status_code);
//     cr_assert_eq(201, response->status_line.status_code);

//     response = request_handling(request, cache);
//     cr_assert_eq(0,strcmp(response->message_body, "jzoef"));
//     printf("code 200 %d\n", response->status_line.status_code);
//     cr_assert_eq(200, response->status_line.status_code);

//     free(response);
// }
// Test(http_method, post, .init=setup, .fini=teardown) {
//     puts("test2");
//     remove("files/test.test");

//     char request[]="POST http://cherokee/files/test.test 1.1\nContent-Type:text/json\n\njzoef";
//     struct Response *response = request_handling(request, cache);
//     printf("code 201 %d\n", response->status_line.status_code);
//     cr_assert_eq(201, response->status_line.status_code);
//     response = request_handling(request, cache);
//     printf("code 409 %d\n", response->status_line.status_code);
//     cr_assert_eq(409, response->status_line.status_code);
//     free(response);
// }
// Test(http_method, get_not_acceptable) {
//     puts("test3");
//     cache = init_cache();
//     char request[] = "GET http://cherokee/files/test.xml 1.1\nContent-Type:text/json\n\njzoef";
//     for (int i = 0; i < LIMIT; i++) {
//         puts(cache[i]);
//     }
//     struct Response *response = request_handling(request, cache);
//     cr_assert_eq(406, response->status_line.status_code);
//     //cr_assert_eq(0,strcmp(response->message_body, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n- <note>\n<to>Tove</to>\n<from>Jani</from>\n<heading>Reminder</heading>\n<body>Don't forget me this weekend!</body>\n</note>"));
//     free(response);
// }
// Test(http_method, get) {
//     puts("test4");
//     cache = init_cache();
//     char request[] = "GET http://cherokee/files/test.xml 1.1\nContent-Type:text/json\nAccept: text/xml\n\njzoef";
//     struct Response *response = request_handling(request, cache);
//     cr_assert_eq(200, response->status_line.status_code);
//     response = request_handling(request, cache);
//     cr_assert_eq(0,strcmp(response->message_body, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n- <note>\n  <to>Tove</to>\n  <from>Jani</from>\n  <heading>Reminder</heading>\n  <body>Don't forget me this weekend!</body>\n</note>"));
//     free(response);
// }

// Test(criterion, test) {
//     cr_assert(1);
// }

void assignMethod(char *method, struct Request *request) {
    strncpy(request->request_line.method, method, 9);
    request->request_line.method[9] = '\0';
}

void assignUri(char *uri, struct Request *request) {
    strncpy(request->request_line.request_uri, uri, 254);
    request->request_line.request_uri[254] = '\0';
}

TestSuite(request_parsing);
Test(request_parsing, get_request_headers_line_and_headers)
{
    char *request_line_and_headers = get_request_headers("GET /files/test.xml 1.1\nContent-Type:text/xml\r\n\r\njzoef");
    cr_assert_str_eq(request_line_and_headers, "GET /files/test.xml 1.1\nContent-Type:text/xml");
    free(request_line_and_headers);
}

Test(request_parsing, get_message_body)
{
    char *message_body = get_message_body("GET http://cherokee/files/test.xml 1.1\nContent-Type:text/json\r\n\r\njzoef");
    cr_assert_str_eq(message_body, "jzoef");
}

Test(request_parsing, get_request_line)
{
    char request_str[] = "GET http://cherokee/files/test.xml 1.1";
    struct Request *request = malloc(sizeof(struct Request));
    parse_request_line(request_str, request);
    cr_assert_str_eq(request->request_line.http_version, "1.1");
    cr_assert_str_eq(request->request_line.method, "GET");
    cr_assert_str_eq(request->request_line.request_uri, "http://cherokee/files/test.xml");
    free(request);
}

Test(request_parsing, get_message_body_length)
{
    char *message_body = get_message_body("GET http://cherokee/files/test.xml 1.1\nContent-Type:text/json\r\n\r\njzoef");
    cr_assert_eq(strlen(message_body), 5);
}

Test(request_parsing, parse_content_type) {
    char h[] = "Content-Type:text/json";
    struct Header *header = malloc(sizeof(struct Header));
    parse_content_type(h, header);
    cr_assert_str_eq(header->content_type_header->media_type, "text/json");
    free(header);
}

TestSuite(request_method_checking);
Test(request_method_checking, is_get_method) {
    struct Request *request_struct = malloc(sizeof(struct Request));
    // request_struct->request_line = malloc(sizeof(struct RequestLine));
    // request_struct->request_line.method = "GET";
    assignMethod("GET", request_struct);
    cr_assert(is_get_method(*request_struct), "expected value was true");
    // request_struct->request_line.method = "POST";
    assignMethod("POST", request_struct);
    cr_assert(!is_get_method(*request_struct), "expected value was false");
    free(request_struct);
}

Test(request_method_checking, is_post_method) {
    struct Request *request_struct = malloc(sizeof(struct Request));
    // request_struct->request_line = malloc(sizeof(struct RequestLine));
    // request_struct->request_line.method = "POST";
    assignMethod("POST", request_struct);
    cr_assert(is_post_method(*request_struct), "expected value was true");
    // request_struct->request_line.method = "GET";
    assignMethod("GET", request_struct);
    cr_assert(!is_post_method(*request_struct), "expected value was false");
    free(request_struct);
}

Test(request_method_checking, is_put_method) {
    struct Request *request_struct = malloc(sizeof(struct Request));
    // request_struct->request_line = malloc(sizeof(struct RequestLine));
    // request_struct->request_line.method = "PUT";
    assignMethod("PUT", request_struct);
    cr_assert(is_put_method(*request_struct), "expected value was true");
    // request_struct->request_line.method = "GET";
    assignMethod("GET", request_struct);
    cr_assert(!is_put_method(*request_struct), "expected value was false");
    free(request_struct);
}

Test(request_method_checking, is_delete_method) {
    struct Request *request_struct = malloc(sizeof(struct Request));
    // request_struct->request_line = malloc(sizeof(struct RequestLine));
    // request_struct->request_line.method = "DELETE";
    assignMethod("DELETE", request_struct);
    cr_assert(is_delete_method(*request_struct), "expected value was true");
    // request_struct->request_line.method = "GET";
    assignMethod("GET", request_struct);
    cr_assert(!is_delete_method(*request_struct), "expected value was false");
    free(request_struct);
}

// TestSuite(cache);
// Test(cache, look_up) {
//     struct LRUCache *lru_cache = init_cache();
//     char *message = look_up("http://cherokee/files/test.xml", lru_cache);
//     cr_assert_eq(message, NULL);
//     free(lru_cache);
// }

Test(generate_response, generateResponseHeader) {
    struct Response *response = malloc(sizeof(struct Response));
    generateResponseHeader("http://cherokee/files/test.xml", 200, "OK", response);
    cr_assert_eq(response->status_line.status_code, 200);
    cr_assert_str_eq(response->status_line.reason_phrase, "OK");
    cr_assert_str_eq(response->header, "application/xml");
    free(response);
}

Test(get_request_handler, should_404_when_no_folder) {
    struct Response *response = malloc(sizeof(struct Response));
    struct Request *request = malloc(sizeof(struct Request));
    // request->request_line = malloc(sizeof(struct RequestLine));
    // request->request_line.method = "GET";
    assignMethod("GET", request);
    // request->request_line.request_uri = "/files/some_folder_that_does_not_exist/";
    assignUri("/files/some_folder_that_does_not_exist/", request);
    get(*request, response);
    cr_assert_eq(response->status_line.status_code, 404);
    cr_assert_str_eq(response->status_line.reason_phrase, "Error");
    free(response);
    free(request);
}

Test(get_request_handler, should_open_folder_that_exists) {
    struct Response *response = malloc(sizeof(struct Response));
    struct Request *request = malloc(sizeof(struct Request));
    // request->request_line = malloc(sizeof(struct RequestLine));
    // request->request_line.method = "GET";
    assignMethod("GET", request);
    // request->request_line.request_uri = "/files/test_folder/";
    assignUri("/files/test_folder/", request);
    int fd = get(*request, response);
    cr_assert(fd > 0, "fd should be positive");
    free(response);
    free(request);
}

Test(get_request_handler, should_404_when_no_file) {
    struct Response *response = malloc(sizeof(struct Response));
    struct Request *request = malloc(sizeof(struct Request));
    // request->request_line = malloc(sizeof(struct RequestLine));
    // request->request_line.method = "GET";
    assignMethod("GET", request);
    // request->request_line.request_uri = "/files/some_file_that_does_not_exist.txt";
    assignUri("/files/some_file_that_does_not_exist.txt", request);
    get(*request, response);
    cr_assert_eq(response->status_line.status_code, 404);
    cr_assert_str_eq(response->status_line.reason_phrase, "Error");
    free(response);
    free(request);
}

Test(get_request_handler, should_open_file_that_exists) {
    struct Response *response = malloc(sizeof(struct Response));
    struct Request *request = malloc(sizeof(struct Request));
    // request->request_line = malloc(sizeof(struct RequestLine));
    // request->request_line.method = "GET";
    assignMethod("GET", request);
    // request->request_line.request_uri = "/files/folder/test.jpg";
    assignUri("/files/folder/test.jpg", request);
    int fd = get(*request, response);
    cr_assert(fd > 0, "fd should be positive");
    free(response);
    free(request);
}

Test(trim_whitespace, should_trim_whitespace) {
    char *str = "   some string with whitespace   ";
    char *trimmed = trim_whitespace(str);
    cr_assert_str_eq(trimmed, "some string with whitespace");
    free(trimmed);
}

Test(extract_extension, should_return_extension) {
    cr_assert_str_eq(extract_extension("text/json"), "json");
    cr_assert_str_eq(extract_extension("text/plain"), "txt");
    cr_assert_str_eq(extract_extension("text/html"), "html");
    cr_assert_str_eq(extract_extension("text/xml"), "xml");
    cr_assert_str_eq(extract_extension("text/css"), "css");
    cr_assert_str_eq(extract_extension("text/javascript"), "js");
    cr_assert_str_eq(extract_extension("image/jpeg"), "jpg");
    cr_assert_str_eq(extract_extension("image/png"), "png");
    cr_assert_str_eq(extract_extension("application/pdf"), "pdf");
}

char *postRelativePath = "files/post_test_folder/new-test-file.txt";

void post_setup(void) {
    if (access("files/post_test_folder/file-that-already-exists.txt", F_OK) == -1) {
        fopen("files/post_test_folder/file-that-already-exists.txt", "w");
    }
    if (access(postRelativePath, F_OK) == -1) {
        perror("File does not exist");
        return;
    }
    if (access(postRelativePath, W_OK) == -1) {
        perror("Insufficient permissions to delete the file");
        return;
    }

    if (remove(postRelativePath) == 0) {
        printf("Deleted successfully in setup\n");
    } else {
        perror("Unable to delete the file in setup\n");
    }
}

TestSuite(post_request_handler, .init=post_setup);
Test(post_request_handler, should_create_empty_text_file) {
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/post_test_folder/new-test-file";
    int fd = createFileWithBody(uri, "txt", 1, response);
    cr_assert(fd > 0, "fd should be positive");
    cr_assert_eq(response->status_line.status_code, 201);
    cr_assert_str_eq(response->status_line.reason_phrase, "Created");
    cr_assert(access(postRelativePath, F_OK) != -1, "File should exist");
    free(response);
}

Test(post_request_handler, should_error_if_file_already_exists) {
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/post_test_folder/file-that-already-exists";
    int fd = createFileWithBody(uri, "txt", 1, response);
    cr_assert(fd < 0, "fd should be negative");
    cr_assert_eq(response->status_line.status_code, 409);
    cr_assert_str_eq(response->status_line.reason_phrase, "File already exists");
}

char* putRelativePath = "files/put_test_folder/new-test-file.txt";
void put_setup(void) {
    if (access("files/put_test_folder/file-that-already-exists.txt", F_OK) == -1) {
        fopen("files/put_test_folder/file-that-already-exists.txt", "w");
    }
    if (access(putRelativePath, F_OK) == -1) {
        perror("File does not exist");
        return;
    }
    if (access(putRelativePath, W_OK) == -1) {
        perror("Insufficient permissions to delete the file");
        return;
    }

    if (remove(putRelativePath) == 0) {
        printf("Deleted successfully in setup\n");
    } else {
        perror("Unable to delete the file in setup\n");
    }
}

TestSuite(put_request_handler, .init=put_setup);
Test(put_request_handler, should_create_empty_text_file_if_it_does_not_exist) {
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/put_test_folder/new-test-file";
    int fd = createFileWithBody(uri, "txt", 0, response);
    cr_assert(fd > 0, "fd should be positive");
    cr_assert_eq(response->status_line.status_code, 201);
    cr_assert_str_eq(response->status_line.reason_phrase, "Created");
    cr_assert(access(putRelativePath, F_OK) != -1, "File should exist");
    free(response);
}

Test(put_request_handler, should_return_ok_if_file_already_exists) {
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/put_test_folder/file-that-already-exists";
    int fd = createFileWithBody(uri, "txt", 0, response);
    cr_assert(fd > 0, "fd should be positive");
    cr_assert_eq(response->status_line.status_code, 200);
    cr_assert_str_eq(response->status_line.reason_phrase, "OK");
    free(response);
}

char* deleteRelativePath = "files/test-file-to-delete.txt";
void delete_setup(void) {
    if (access(deleteRelativePath, F_OK) == -1) {
        fopen(deleteRelativePath, "w");
    }
}

TestSuite(delete_request_handler, .init=delete_setup);
Test(delete_request_handler, should_delete_file_that_already_exists) {
    struct Request *request = malloc(sizeof(struct Request));
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/test-file-to-delete.txt";
    // request->request_line.request_uri = uri;
    assignUri(uri, request);
    delete(*request, response);
    cr_assert_eq(response->status_line.status_code, 200);
    cr_assert_str_eq(response->status_line.reason_phrase, "OK");
    free(response);
}

Test(delete_request_handler, should_404_if_file_does_not_exist) {
    struct Request *request = malloc(sizeof(struct Request));
    struct Response *response = malloc(sizeof(struct Response));
    char uri[] = "/files/file-that-does-not-exist.txt";
    // request->request_line.request_uri = uri;
    assignUri(uri, request);
    delete(*request, response);
    cr_assert_eq(response->status_line.status_code, 404);
    cr_assert_str_eq(response->status_line.reason_phrase, "Delete failed");
    free(response);
}