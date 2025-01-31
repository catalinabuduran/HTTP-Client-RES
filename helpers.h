#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

void error(const char *msg);
int contains_non_alphanumeric(const char *title);
void compute_message(char *message, const char *line);
int open_connection(char *host_ip, int portno, int ip_type, int socket_type,
                    int flag);
void close_connection(int sockfd);
void send_to_server(int sockfd, char *message);
char *receive_from_server(int sockfd);
int get_id();
char *get_book();
char *read_credentials();
char *basic_extract_json_response(char *str);


#endif
