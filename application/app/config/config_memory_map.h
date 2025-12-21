#ifndef CONFIG_MEMORY_MAP_H
#define CONFIG_MEMORY_MAP_H



/*
FRAM 8KB
┌───────────────────┬────────────┬────────────┬────────────┐
│ Name              │ Start      │ Size       │ End        │
├───────────────────┼────────────┼────────────┼────────────┤
│ CONFIG            │ 0x00000000 │ 0x00000800 │ 0x000007FF │
│ CONFIG_SENSOR     │ 0x00000800 │ 0x00000800 │ 0x00000FFF │
│ CONFIG_CALI       │ 0x00001000 │ 0x00000800 │ 0x000017FF │
│ CONFIG_NVM        │ 0x00001800 │ 0x00000400 │ 0x00001BFF │
│ CONFIG_JOURNAL    │ 0x00001C00 │ 0x00000400 │ 0x00001FFF │
└───────────────────┴────────────┴────────────┴────────────┘
*/

#define CONFIG_START_ADDRESS        0x00000000 
#define CONFIG_MEMORY_SIZE          0x00000800 //2048

#define CONFIG_SENSOR_START_ADDRESS 0x00000800 
#define CONFIG_SENSOR_MEMORY_SIZE   0x00000800  //2048

#define CONFIG_CALI_START_ADDRESS   0x00001000 
#define CONFIG_CALI_MEMORY_SIZE     0x00000800 //2048

#define CONFIG_NVM_START_ADDRESS    0x00001800 
#define CONFIG_NVM_MEMORY_SIZE      0x00000400   //1024

#define CONFIG_JOURNAL_START_ADDRESS    0x00001C00 
#define CONFIG_JOURNAL_MEMORY_SIZE      0x00000400 //1024

#endif

