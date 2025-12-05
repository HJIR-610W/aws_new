

#include "const_string.h"
#include "config_app.h"



const char* panel_list_eng[] = {
#define X(code, name) name,
    PANEL_LIST
#undef X
};


const char* panel_item6_type_list_eng[] = {
#define X(code, name) name,
    PANEL_ITEM6_TYPE_LIST
#undef X
};


const char* baud_list_eng[] = {
#define X(code, name) name,
    BAUD_LIST
#undef X
};



const char* cdma_model_list_eng[] = {
#define X(code, name) name,
    CDMA_MODEL_LIST
#undef X
};

const char* rain_mm_list_eng[] = {
#define X(code, name) name,
    RAIN_MM_LIST
#undef X
};


const char* enable_list_eng[] = {"Disabled", "Enabled"};
const char* eth_mode_list_eng[] = {"Client", "Server"};

const char* protocol_list_eng[] = {"KMA2", "KMA3"};

const char *adc_se_list[18] = {"SE  0", "SE  1", "SE  2", "SE  3", "SE  4", "SE  5", "SE  6", "SE  7",
                               "SE  8", "SE  9", "SE 10", "SE 11", "SE 12", "SE 13", "SE 14", "SE 15", "PT100 A", "PT100 B"};

const char *adc_single_list[16] = {"SE  0", "SE  1", "SE  2", "SE  3", "SE  4", "SE  5", "SE  6", "SE  7",
                                "SE  8", "SE  9", "SE 10", "SE 11", "SE 12", "SE 13", "SE 14", "SE 15"};

const char* adc_diff_list[8] = {"DIF 0", "DIF 1", "DIF 2", "DIF 3","DIF 4", "DIF 5", "DIF 6", "DIF 7"};


const char *freq_ch_list[2] = {"FREQ 1", "FREQ 2"};
const char *pt100_list_eng[2] = {"PT100 0", "PT100 1"};
const char *charger_list_kor[3] = {"미사용","화진 스마트", "LS1024"};
const char *charger_list_eng[3] = {"Not Used", "SMART", "LS1024"};


const char *door_status_list_eng[2] = {"CLOSED", "OPENED"};
const char *door_status_list_kor[2] = {"닫힘", "열림"};

const char *link_status_list_eng[3] = {"-", "UP", "DOWN"};
const char *ethlink_status_list_eng[3] = {"-", "U", "D"};

const char *sdcard_status_list_eng[] = {"NOT INSERTED", "INSERTED"};

const char *lcd_off_time_list_eng[] = {"10", "60", "Always On"};




const char *safe_name(const char **names,int name_count,int index)
{
  const char *str="unknown";

  if(index>=name_count)
  {
    return str;
  }

  if(names[index]==0)
  {
    return str;
  }

  return names[index];

}