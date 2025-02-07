#ifndef MODEM_IF_H
#define MODEM_IF_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "time_define.h"
    typedef uint16_t M_RET_t;

#define RET_OK         (0U)
#define RET_FAIL       (1U)
#define RET_ERR        (2U)
#define RET_SERVER_ERR (3U)
#define RET_MODEM_ERR  (4U)
#define RET_TIME_OUT   (5U)
#define RET_FAIL_RESP  (6U)
#define RET_FAIL_SEND  (7U)

#define M_RESET_SW (0U)
#define M_RESET_HW (1U)


    typedef struct
    {
        DATE_TIME_BUF time;
        char num[12];  //01011111111
        char msg[200];
    }sms_t;



  
/// @struct iCellular_t
/// @brief 셀룰러 모뎀 인터페이스
typedef struct iCellular
{
    /**
     * @brief 모뎀 리셋 후 지연시간
     */
    uint32_t resetDelay;            
    
    /**
     * @brief 모뎀 부팅이 완료되면 초기화 해줘야 하는것들 모음
     * 
     * @return 초기화 결과
     */
    M_RET_t (*init)(void);
    M_RET_t (*send_sms)(char *num,char *msg);            // SMS 전송
    M_RET_t (*read_sms)(sms_t *pSms);           // 가장 최신에 수신된 SMS 읽기
    M_RET_t (*read_num)(char *prNum,uint16_t numSize);          // 모뎀 번화번호 읽기
    M_RET_t (*read_rssi)(int16_t *);            // 수신감도 
    
    /**
     * @brief tcp 서버 정보
     * @param ip 서버 ip 
     * @param port 서버 포트
     */
    void    (*write_ip)(uint8_t ip[4],uint16_t port);  
    M_RET_t (*open_tcp)(void);                           // tcp 소켓 열기
    M_RET_t (*close_tcp)(void);                          // tcp 소켓 닫기
    M_RET_t (*open_ppp)(void);                           // ppp 열기
    M_RET_t (*close_ppp)(void);                          // ppp 닫기 
    M_RET_t (*send_tcp)(uint8_t *data,uint16_t dataLen); // tcp 데이터 전송
    
    /**
     * @brief tcp 데이터 수신
     * @param buff 수신버퍼
     * @param buffSize 버퍼 사이즈
     * @param recvLen  수신된 데이터 길이
     * @param timeOutMs 수신 대기 시간
     * 
     * @return M_RET_t
     */
    M_RET_t (*recv_tcp)(uint8_t *buff,uint16_t buffSize,uint16_t *recvLen,uint32_t timeOutMs); // tcp 데이터 수신
    
    /**
     * @brief 모뎀을 리셋함
     * @param resetType 리셋의 종류
     * @param delayMs   리셋 후 지연시간(모뎀 안정화 시간(필요시))
     */
    void (*reset)(uint8_t resetType,uint32_t delayMs);

    /**
     * @brief 모뎀에서 수신된 문자열에서 dtmf만 추출
     * @param data 수신된 문자열
     * 
     * @return dtmf(예 '1')
     *  
     */
    char (*get_dtmf)(char *data);
    
    void (*vpn_init)(void);// VPN 초기화
    
    /**
     * @brief 모뎀전원을 차단하기전에 안정적인 모뎀 사용을 위해 실행해줘야하는 at 명령어
     */
    void (*off_powerSafe)(void);
    /**
     * @brief 전화건 사람 번호 추출
     */
    M_RET_t (*read_ringNum)(char *pData,char *prNum,uint16_t numSize); 
    M_RET_t (*recv_call)(void);

    M_RET_t (*dial)(char *num,uint32_t waitTimeOutMs);

    M_RET_t (*set_vpn_config)(char *id,char *pw,uint8_t ip[4],uint16_t port);
    M_RET_t (*read_vpn_config)(char *outBuffer,uint16_t outSize);
    M_RET_t (*at_direct)(char *at,char *outBuffer,uint16_t outSize);

    M_RET_t (*check_network_service)(char *msgOut,uint16_t msgSize);
}iCellular_t;


    typedef struct mqtt_if_s
    {
        uint32_t resetDelay;
        void (*init)(void);
        void (*reset)(uint8_t resetType, uint32_t delayMs);
        M_RET_t(*open)(void);
        M_RET_t(*close)(void);
        M_RET_t(*subcribe)(const char* topic);
        M_RET_t(*recv)(uint32_t port,char* data, uint16_t dataLen);
        M_RET_t(*publish)(const char* topic, const char* payLoad);
        M_RET_t(*recv_topic)(char* buff,uint32_t buffSize, uint32_t timeOutms,uint32_t *rLen);
        void (*write_user)(char* id, char* user, char* pw);
        void    (*write_ip)(uint8_t ip[4], uint16_t port);
    }iMqtt_t;

#ifdef __cplusplus
}
#endif

#endif

