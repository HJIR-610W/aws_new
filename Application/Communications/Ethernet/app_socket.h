

#ifndef APP_SOCKET_H
#define APP_SOCKET_H

void enable_keepalive(int sock, int idle_time, int interval, int max_probes);
#endif