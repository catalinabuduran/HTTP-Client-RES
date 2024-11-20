#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "parson.h"
#include "helpers.h"
#include "requests.h"


void print_error_message(char *response) {
    JSON_Value *token_value = json_parse_string(basic_extract_json_response(response));
    JSON_Object *token_object = json_value_get_object(token_value);
    const char *token = json_object_get_string(token_object, "error");
    printf("%s\n", token);
}

int main(void) {
    char *ip = "34.246.184.49";
    char command[500];
    char login_cookie[500] = "\0";
    char library_jwt[500] = "\0";
    int sockfd;
    char *cookies[10];
    char *body_data[10];
    int login = 0;
    int access_library = 0;
    char *message;
    char *serialized_string;
    char *response;

    while (1) {
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Error reading command");
            continue;
        }
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "register") == 0) {
            serialized_string = read_credentials();
            body_data[0] = serialized_string;
            if (serialized_string == NULL) {
                login = 0;
                access_library = 0;
                continue;
            }
            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }
            message = compute_post_request(ip, "/api/v1/tema/auth/register",
                                        "application/json", body_data, 1,
                                        NULL, 0, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);

            if (strstr(response, "400 Bad Request") != NULL) {
                char* token_value = basic_extract_json_response(response);
                printf("%s\n", token_value);
            } else {
                printf("Inregistrare efectuata cu SUCCES.\n");
            }
        }

        if (strcmp(command, "login") == 0) {
            serialized_string = read_credentials();
            body_data[0] = serialized_string;
            if (serialized_string == NULL) {
                login = 0;
                access_library = 0;
                continue;
            }

            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }
            message = compute_post_request(ip, "/api/v1/tema/auth/login",
                                        "application/json", body_data, 1,
                                        NULL, 0, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);

            if (strstr(response, "400 Bad Request") != NULL) {
                char* token_value = basic_extract_json_response(response);
                printf("%s\n", token_value);
                access_library = 0;
            } else {
                printf("Login efectuat cu SUCCES.\n");
                int i;
                // extrag cookie
                char *aux = strstr(response, "connect.sid");
                for (i = 0; aux[i] != ';'; i++) {
                    login_cookie[i] = aux[i];
                }
                login_cookie[i] = '\0';
                login = 1;
            }
        }

        if (strcmp(command, "enter_library") == 0) {
            cookies[0] = login_cookie;
            if (cookies[0] == NULL || body_data[0] == NULL) {
                printf("ERROR: You are not logged in.\n");
                continue;
            }
            
            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }
            message = compute_get_request(ip, "/api/v1/tema/library/access",
                                        NULL, cookies, 1, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);

            if (strstr(response, "401 Unauthorized") != NULL) {
                char* token_value = basic_extract_json_response(response);
                printf("%s\n", token_value);
            } else {
                printf("SUCCESS: Acces permis in biblioteca.\n");
                access_library = 1;
                // extragem token JWT
                JSON_Value *token_value = json_parse_string(basic_extract_json_response(response));
                JSON_Object *token_object = json_value_get_object(token_value);
                const char *token = json_object_get_string(token_object, "token");
                strcpy(library_jwt, token);
            }
        }

        if (strcmp(command, "get_books") == 0) {
            if (login == 0 || access_library == 0) {
                printf("ERROR: Acces nepermis\n");
                // continue;  
            } else {
                sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
                if (sockfd == -1) {
                    continue;
                }
                message = compute_get_request(ip, "/api/v1/tema/library/books",
                                            NULL, NULL, 0, 1, library_jwt);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                close(sockfd);

                if (strstr(response, "error") != NULL) {
                    print_error_message(response);
                } else {
                    JSON_Value *token_value = json_parse_string(strstr(response, "["));
                    char *books = json_serialize_to_string_pretty(token_value);
                    printf("%s\n", books);
                }
            }
        }


        if (strcmp(command, "add_book") == 0) {
            if (login == 0 || access_library == 0) {
                printf("ERROR: Acces nepermis\n");
                continue;
            }

            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }
            
            char *serialized_string = get_book();
            if (serialized_string == NULL) {
                close(sockfd);
                continue;
            }
            body_data[0] = serialized_string;

            message = compute_post_request(ip, "/api/v1/tema/library/books",
                                        "application/json", body_data, 1,
                                        NULL, 0, 1, library_jwt);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
                
            if (strstr(response, "error") != NULL) {
                char* token = basic_extract_json_response(response);
                printf("%s\n", token);
            } else {
                printf("Carte adaugata cu SUCCES.\n");
            }
        }

        if (strcmp(command, "get_book") == 0) {
            if (login == 0 || access_library == 0) {
                printf("ERROR: Acces nepermis\n");
                continue;
            }

            char url[100];
            int id;
            id = get_id(); //id-ul cartii
            sprintf(url, "/api/v1/tema/library/books/%d", id);
            
            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }

            message = compute_get_request(ip, url, NULL, NULL, 0, 1, library_jwt);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "error") != NULL) {
                print_error_message(response);
            } else {
                JSON_Value *token_value = json_parse_string(strstr(response, "{"));
                char *book = json_serialize_to_string_pretty(token_value);
                printf("%s\n", book);
            }
            close(sockfd);
        }

        if (strcmp(command, "delete_book") == 0) {
            if (login == 0 || access_library == 0) {
                printf("ERROR: Acces nepermis\n");
                continue;
            }
            
            char url[100];
            int id;
            id = get_id();
            sprintf(url, "/api/v1/tema/library/books/%d", id);
            
            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }

            message = compute_delete_request(ip, url, NULL, NULL, 0, 1, library_jwt);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "error") != NULL) {
                char* token_value = basic_extract_json_response(response);
                printf("%s\n", token_value);
            } else {
                printf("Cartea a fost ștearsă cu SUCCES.\n");
            }
            close(sockfd);
        }

        if (strcmp(command, "logout") == 0) {
            if (login == 0) {
                printf("ERROR: You are not logged in.\n");
                continue;
            }
            
            cookies[0] = login_cookie;
            message = compute_get_request(ip, "/api/v1/tema/auth/logout", NULL,
                                        cookies, 1, 0, NULL);
            
            sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                continue;
            }
            
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "error") != NULL) {
                print_error_message(response);
            } else {
                printf("Logout efectuat cu SUCCES.\n");
                memset(login_cookie, 0, 500);
                memset(library_jwt, 0, 500);
                access_library = 0;
                login = 0;
            }
            close(sockfd);
        }

        if (strcmp(command, "exit") == 0) {
            break;
        }
    }
    return 0;
}