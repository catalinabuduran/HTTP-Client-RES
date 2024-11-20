#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> 

#include "parson.h"
#include "helpers.h"
#include "buffer.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

int contains_space(const char *str) {
    while (*str) {
        if (*str == ' ') {
            return 1;
        }
        str++;
    }
    return 0;
}

int get_id() {
    char id_book[200];
    int is_number;
    is_number = 0;
    printf("id=");
    fgets(id_book, 200, stdin);
    id_book[strlen(id_book) - 1] = '\0';
    is_number = 0;
    for (unsigned int i = 0; i < strlen(id_book); i++) {
        if (id_book[i] < '0' || id_book[i] > '9') {
            is_number = 1;
        }
    }
    if (is_number == 1) {
        printf("ERROR: invalid value of id\n");
    }
    int id;
    id = atoi(id_book);
    return id;
}

int contains_non_alphanumeric(const char *title) {
    while (*title) {
        if (!isalnum(*title)) {
            return 1;
        }
        title++;
    }
    return 0;
}

char *get_book() {
    char title[500], author[500], genre[500], page_count[500], publisher[500];
    char *string;

    printf("title=");
    fgets(title, 500, stdin);
    title[strcspn(title, "\n")] = '\0';

    printf("author=");
    fgets(author, 500, stdin);
    author[strcspn(author, "\n")] = '\0';

    printf("genre=");
    fgets(genre, 500, stdin);
    genre[strcspn(genre, "\n")] = '\0';

    printf("publisher=");
    fgets(publisher, 500, stdin);
    publisher[strcspn(publisher, "\n")] = '\0';

    printf("page_count=");
    fgets(page_count, 50, stdin);
    page_count[strcspn(page_count, "\n")] = '\0';

    if (title[0] == '\0') {
        printf("ERROR:Invalid title\n");
        return NULL;
    }
    //am comentat aceasta parte de cod pentru ca la rularea checkerului, aveam ca output
    //la toate comenzile add_book => "non alfanumeric", la rulare manuala, era totul ok

    // if (contains_non_alphanumeric(title)) {
    //     printf("ERROR:non alfanumeric\n");
    //     return NULL;
    // }

    if (author[0] == '\0') {
        printf("ERROR:Invalid author\n");
        return NULL;
    }

    if (genre[0] == '\0') {
        printf("ERROR:Invalid genre\n");
        return NULL;
    }

    if (publisher[0] == '\0') {
        printf("ERROR:Invalid publisher\n");
        return NULL;
    }

    int is_number = 0;
    for (unsigned int i = 0; i < strlen(page_count); i++) {
        if (page_count[i] < '0' || page_count[i] > '9') {
            is_number = 1;
            break;
        }
    }

    if (is_number == 1) {
        printf("ERROR:Invalid page count, enter a valid value\n");
        return NULL;
    }

    string = (char *)malloc(1000 * sizeof(char));
    if (string == NULL) {
        printf("ERROR: Memory allocation failed\n");
        return NULL;
    }

    strcpy(string, "{\"title\": \"");
    strcat(string, title);
    strcat(string, "\", \"author\": \"");
    strcat(string, author);
    strcat(string, "\", \"genre\": \"");
    strcat(string, genre);
    strcat(string, "\", \"page_count\": ");
    strcat(string, page_count);
    strcat(string, ", \"publisher\": \"");
    strcat(string, publisher);
    strcat(string, "\"}");

    return string;
}


char *read_credentials() {
    char username[100], password[100];
    printf("username=");
    fgets(username, 100, stdin);
    printf("password=");
    fgets(password, 100, stdin);
    username[strcspn(username, "\n")] = '\0';
    password[strcspn(password, "\n")] = '\0';

    if (contains_space(username)) {
        fprintf(stderr, "Error: Username should not contain spaces.\n");
        return NULL;
    }

    if (contains_space(password)) {
        fprintf(stderr, "Error: Password should not contain spaces.\n");
        return NULL;
    }

    size_t total_length;
    total_length = strlen("{\"username\":\"\",\"password\":\"\"}") + strlen(username) + strlen(password) + 1;
    char *credentials;
    credentials = malloc(total_length * sizeof(char));

    if (credentials == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    snprintf(credentials, total_length, "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);
    return credentials;
}