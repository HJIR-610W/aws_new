
#ifndef HTTP_WEBSOCKET_SERVER_H
#define HTTP_WEBSOCKET_SERVER_H


void send_html(int conn);
void send_icon(int conn);
void handle_websocket_handshake(int conn, const char *request) ;
void handle_websocket_connection(int conn) ;
#endif