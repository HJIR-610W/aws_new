
#ifndef ADS1220_REG_H
#define ADS1220_REG_H


#define ADS1220_REG_0   0x00
#define ADS1220_REG_1   0x01
#define ADS1220_REG_2   0x02
#define ADS1220_REG_3   0x03


/* Command Definitions */
#define ADS1220_CMD_RDATA       0x10
#define ADS1220_CMD_RREG        0x20
#define ADS1220_CMD_WREG        0x40
#define ADS1220_CMD_SYNC        0x08
#define ADS1220_CMD_SHUTDOWN    0x02
#define ADS1220_CMD_RESET       0x06



#define b00000001   0x01
#define b00000010   0x02
#define b00000100   0x04
#define b00001000   0x08
#define b00010000   0x10
#define b00100000   0x20
#define b01000000   0x40
#define b10000000   0x80


//레지스터 0
#define ADS1220_MUX_AIN0_AIN1  0x00
#define ADS1220_MUX_AIN0_AIN2  0x10
#define ADS1220_MUX_AIN0_AIN3  0x20
#define ADS1220_MUX_AIN1_AIN2  0x30
#define ADS1220_MUX_AIN1_AIN3  0x40
#define ADS1220_MUX_AIN2_AIN3  0x50


#define ADS1220_GAIN_1      0x00
#define ADS1220_GAIN_2      0x02
#define ADS1220_GAIN_4      0x04
#define ADS1220_GAIN_8      0x06
#define ADS1220_GAIN_16     0x08
#define ADS1220_GAIN_32     0x0a
#define ADS1220_GAIN_64     0x0c
#define ADS1220_GAIN_128    0x0e

#define ADS1220_PGA_ENABLE    0
#define ADS1220_PGA_DISABLE   1



//레지스터 1
#define REG1_DR_20           0x00
#define REG1_DR_45           0x20
#define REG1_DR_90           0x40
#define REG1_DR_175          0x60
#define REG1_DR_330          0x80
#define REG1_DR_600          0xa0
#define REG1_DR_1000         0xc0


#define REG1_MODE_NORMAL 0x00
#define REG1_MODE_DUTY   0x08
#define REG1_MODE_TURBO  0x10
#define REG1_MODE_DCT    0x18

#define REG1_CM_SINGLE   0x00
#define REG1_CM_CONTINUE 0x04

#define REG1_TEMP_SENSOR_DISABLE 0x00
#define REG1_TEMP_SENSOR_ENABLE  0x02

#define REG1_BCS_OFF         0x00
#define REG1_BCS_ON         0x01

//레즈스터 2
#define REG2_VREF_INT    0x00
#define REG2_VREF_EX_DED 0x40
#define REG2_VREF_EX_AIN 0x80
#define REG2_VREF_SUPPLY 0xc0


#define ADS1220_REJECT_OFF  0x00
#define ADS1220_REJECT_BOTH 0x10
#define ADS1220_REJECT_50   0x20
#define ADS1220_REJECT_60   0x30

#define REG2_PSW_AUTO      0x08

#define ADS1220_IDAC_OFF    0x00
#define ADS1220_IDAC_10     0x01
#define ADS1220_IDAC_50     0x02
#define ADS1220_IDAC_100    0x03
#define ADS1220_IDAC_250    0x04
#define ADS1220_IDAC_500    0x05
#define ADS1220_IDAC_1000   0x06
#define ADS1220_IDAC_2000   0x07

//레지스터 3
#define ADS1220_IDAC1_OFF   0x00
#define ADS1220_IDAC1_AIN0  0x20
#define ADS1220_IDAC1_AIN1  0x40
#define ADS1220_IDAC1_AIN2  0x60
#define ADS1220_IDAC1_AIN3  0x80
#define ADS1220_IDAC1_REFP0 0xa0
#define ADS1220_IDAC1_REFN0 0xc0

#define ADS1220_IDAC2_OFF   0x00
#define ADS1220_IDAC2_AIN0  0x04
#define ADS1220_IDAC2_AIN1  0x08
#define ADS1220_IDAC2_AIN2  0x0c
#define ADS1220_IDAC2_AIN3  0x10
#define ADS1220_IDAC2_REFP0 0x14
#define ADS1220_IDAC2_REFN0 0x18

#define ADS1220_DRDY_MODE   0x02


#define ADS1220_CMD_RESET       0x06 /* 레지스터값 리셋됨, 이명령이우 50us +32*tclk 지연 필요*/
#define ADS1220_CMD_START_SYNC  0x08 /* single shot 모드에서 사용*/
#define ADS1220_CMD_POWERDOWN   0x02
#define ADS1220_CMD_RDATA       0x10
#define ADS1220_CMD_RREG        0x20
#define ADS1220_CMD_WREG        0x40

#endif