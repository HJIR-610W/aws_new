


#include "config\migration\config_app_migration.h"
#include "config_app.h"
#include <string.h>


void config_app_migration(uint32_t current_version,uint32_t target_version,void *old_config)
{

    if(current_version==1 && target_version==2)
    {
        //v1 -> v2 마이그레이션 처리
        config_app_v1_t *p_old_config = (config_app_v1_t *)old_config;
        config_t *p_new_config = get_config_app();
        
        //공통 필드 복사
        memcpy(p_new_config, p_old_config, offsetof(config_app_v1_t, com_encrypt_active));
        
        //v1의 vhf 필드를 건너뛰고 나머지 필드 복사
        p_new_config->com_encrypt_active = p_old_config->com_encrypt_active;
        p_new_config->cdma_vpn_active = p_old_config->cdma_vpn_active;
        p_new_config->ac_active = p_old_config->ac_active;
        p_new_config->dev_telnet_mode = p_old_config->dev_telnet_mode;
        memcpy(p_new_config->dev_telnet_ip, p_old_config->dev_telnet_ip, sizeof(p_new_config->dev_telnet_ip));
        p_new_config->dev_telnet_port = p_old_config->dev_telnet_port;
        p_new_config->lcd_off_time_index = p_old_config->lcd_off_time_index;
        p_new_config->aws_csv_save_active = p_old_config->aws_csv_save_active;
        
        //v2에서 위치가 변경된 vhf 필드 복사
        p_new_config->vhf_id = p_old_config->vhf_id;
        p_new_config->vhf_group = p_old_config->vhf_group;
        p_new_config->vhf_host_id = p_old_config->vhf_host_id;
        p_new_config->vhf_repeater_id = p_old_config->vhf_repeater_id;
        p_new_config->vhf_ptt_delay = p_old_config->vhf_ptt_delay;
        save_config_app();

    }
    else if(current_version==2 && target_version==1)
    {
        //v2 -> v1 마이그레이션 처리
        config_app_v1_t *p_new_config = (config_app_v1_t *)old_config;
        config_t *p_old_config = get_config_app();
        
        //공통 필드 복사
        memcpy(p_new_config, p_old_config, offsetof(config_app_v1_t, com_encrypt_active));
        
        //v1의 vhf 필드를 건너뛰고 나머지 필드 복사
        p_new_config->com_encrypt_active = p_old_config->com_encrypt_active;
        p_new_config->cdma_vpn_active = p_old_config->cdma_vpn_active;
        p_new_config->ac_active = p_old_config->ac_active;
        p_new_config->dev_telnet_mode = p_old_config->dev_telnet_mode;
        memcpy(p_new_config->dev_telnet_ip, p_old_config->dev_telnet_ip, sizeof(p_new_config->dev_telnet_ip));
        p_new_config->dev_telnet_port = p_old_config->dev_telnet_port;
        p_new_config->lcd_off_time_index = p_old_config->lcd_off_time_index;
        p_new_config->aws_csv_save_active = p_old_config->aws_csv_save_active;
        
        //v2에서 위치가 변경된 vhf 필드 복사
        p_new_config->vhf_id = p_old_config->vhf_id;
        p_new_config->vhf_group = p_old_config->vhf_group;
        p_new_config->vhf_host_id = p_old_config->vhf_host_id;
        p_new_config->vhf_repeater_id = p_old_config->vhf_repeater_id;
        p_new_config->vhf_ptt_delay = p_old_config->vhf_ptt_delay;
        save_config_app();
    }

}