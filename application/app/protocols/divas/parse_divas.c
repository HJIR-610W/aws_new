#include "parse_divas.h"

#include "divas_protocol_define.h"
#include "system_err.h"

void parse_divas_request_cmd1(uint8_t *p_data, size_t data_len)
{

    TASK_PRINTF("Parsing DIVAS CMD 1 request.\n");
}

void parse_divas_request(uint8_t *p_data, size_t data_len)
{
    divas_frame_t *p_frame = (divas_frame_t *)p_data;

    switch(p_frame->CMD)
    {
        case DIVAS_CMD_RD_INDEX:
            TASK_PRINTF("Parsing DIVAS_CMD_RD_INDEX request.\n");
            parse_divas_request_cmd1(p_data, data_len);
            break;

        case DIVAS_CMD_FW_DOWNLOAD:
            TASK_PRINTF("Parsing DIVAS_CMD_FW_DOWNLOAD request.\n");

            break;
        case DIVAS_CMD_FW_UPDATE:
            TASK_PRINTF("Parsing DIVAS_CMD_FW_UPDATE request.\n");
        break;
        case DIVAS_CMD_RD_SYSTEM:
            TASK_PRINTF("Parsing DIVAS_CMD_RD_SYSTEM request.\n");
     
            break;
        case DIVAS_CMD_RESET:
            TASK_PRINTF("Parsing DIVAS_CMD_RESET request.\n");

   
            break;
        case DIVAS_CMD_RD_SYSLOG:
            TASK_PRINTF("Parsing DIVAS_CMD_RD_SYSLOG request.\n");
   
            break;
        case DIVAS_CMD_RD_VERSION:
            TASK_PRINTF("Parsing DIVAS_CMD_RD_VERSION request.\n");
 
            break;
        case DIVAS_CMD_RD_CFG_OFS:
            TASK_PRINTF("Parsing DIVAS_CMD_RD_CFG_OFS request.\n");

            break;
        case DIVAS_CMD_WR_CFG_OFS:
            TASK_PRINTF("Parsing DIVAS_CMD_WR_CFG_OFS request.\n");

            break;


        default:

            break;
    }
}