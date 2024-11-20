#include "requests.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.h"

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, int auth,
                          char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = NULL;
    char *token_line = NULL;

    if (query_params != NULL) {
        line = calloc(LINELEN, sizeof(char));
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
        compute_message(message, line);
        free(line);
    } else {
        line = calloc(LINELEN, sizeof(char));
        sprintf(line, "GET %s HTTP/1.1", url);
        compute_message(message, line);
        free(line);
    }

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    free(line);

    if (auth == 1) {
        token_line = calloc(LINELEN, sizeof(char));
        sprintf(token_line, "Authorization: Bearer %s", token);
        compute_message(message, token_line);
        free(token_line);
    }

    if (cookies != NULL) {
        line = calloc(LINELEN, sizeof(char));
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
        free(line);
    }

    compute_message(message, "");

    return message;
}


char *compute_delete_request(char *host, char *url, char *query_params,
                             char **cookies, int cookies_count, int auth,
                             char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = NULL;
    char *token_line = NULL;

    if (query_params != NULL) {
        line = calloc(LINELEN, sizeof(char));
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
        compute_message(message, line);
        free(line);
    } else {
        line = calloc(LINELEN, sizeof(char));
        sprintf(line, "DELETE %s HTTP/1.1", url);
        compute_message(message, line);
        free(line);
    }

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    free(line);

    if (auth == 1) {
        token_line = calloc(LINELEN, sizeof(char));
        sprintf(token_line, "Authorization: Bearer %s", token);
        compute_message(message, token_line);
        free(token_line);
    }

    if (cookies != NULL) {
        line = calloc(LINELEN, sizeof(char));
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
        free(line);
    }

    compute_message(message, "");

    return message;
}


char *compute_post_request(char *host, char *url, char *content_type,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count, int auth,
                           char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = NULL;
    char *token_line = NULL;
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    free(line);

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    free(line);

    if (auth == 1) {
        token_line = calloc(LINELEN, sizeof(char));
        sprintf(token_line, "Authorization: Bearer %s", token);
        compute_message(message, token_line);
        free(token_line);
    }

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    free(line);

    size_t total_body_data_length = 0;
    for (int i = 0; i < body_data_fields_count; ++i) {
        total_body_data_length += strlen(body_data[i]);
        if (i < body_data_fields_count - 1) {
            total_body_data_length++;
        }
    }

    line = calloc(LINELEN, sizeof(char));
    sprintf(line, "Content-Length: %zu", total_body_data_length);
    compute_message(message, line);
    free(line);

    if (cookies != NULL) {
        line = calloc(LINELEN, sizeof(char));
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
        free(line);
    }

    compute_message(message, "");

    for (int i = 0; i < body_data_fields_count; ++i) {
        strcat(body_data_buffer, body_data[i]);
        if (i < body_data_fields_count - 1) {
            strcat(body_data_buffer, "&");
        }
    }
    compute_message(message, body_data_buffer);
    free(body_data_buffer);

    return message;
}