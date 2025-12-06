
#include "sockets.h"
#include "debug_io.h"

void enable_keepalive(int sock, int idle_time, int interval, int max_probes) {
    int optval = 1;

    // Keep-Alive 활성화
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        dbg_printf("Failed to enable Keep-Alive\n");
        return;
    }

    // 유휴 시간 설정 (첫 번째 Keep-Alive 패킷 전 대기 시간)
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle_time, sizeof(idle_time)) < 0) {
        dbg_printf("Failed to set Keep-Alive idle time\n");
    }

    // Keep-Alive 패킷 간격
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
        dbg_printf("Failed to set Keep-Alive interval\n");
    }

    // Keep-Alive 패킷 전송 실패 허용 횟수
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &max_probes, sizeof(max_probes)) < 0) {
        dbg_printf("Failed to set Keep-Alive probe count\n");
    }
}