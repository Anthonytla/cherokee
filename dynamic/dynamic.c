#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_one(int num) {
    return num + 1;
}

int multiply_ten(int num) {
    return num * 10;
}

char *first_char(const char *text) {
    char *result = malloc(2);
    result[0] = text[0];
    result[1] = '\0';
    return result;
}

char *add_two_from_text(const char *text) {
    int num = atoi(text);
    int result = num + 2;
    char *result_text = malloc(10);
    sprintf(result_text, "%d", result);
    return result_text;
}

char *multiply_ten_from_text(const char *text) {
    int num = atoi(text);
    int result = num * 10;
    char *result_text = malloc(10);
    sprintf(result_text, "%d", result);
    return result_text;
}

char *add_five_from_text(const char *text) {
    int num = atoi(text);
    int result = num + 5;
    char *result_text = malloc(10);
    sprintf(result_text, "%d", result);
    return result_text;
}