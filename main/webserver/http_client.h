#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

void wifi_init_sta(void);
void start_web_client(void);
void http_connect_get(
    const char* url, 
    http_event_handle_cb http_event_handler);
void cleanup_web_client(void);

#endif // HTTP_CLIENT_H
