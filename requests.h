#ifndef _REQUESTS_
#define _REQUESTS_

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, int auth, char *token);
char *compute_post_request(char *host, char *url, char *content_type,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count, int auth,
                           char *tok);
char *compute_delete_request(char *host, char *url, char *query_params,
                             char **cookies, int cookies_count, int auth,
                             char *tok);

#endif
