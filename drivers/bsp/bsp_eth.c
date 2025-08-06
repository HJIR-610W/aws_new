

#include "pcb_define.h"

#include "config_app.h"

extern ETH_HandleTypeDef heth;

extern ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
extern ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */


void bsp_eth_init(void)
{

#if 0 
  int32_t PHYLinkState = 0;
  ETH_MACConfigTypeDef MACConf = {0};
  /* Start ETH HAL Init */
  uint8_t *p_mac = get_config_app()->eth_mac;
  uint8_t MACAddr[6];
  heth.Instance = ETH;
  MACAddr[0] = p_mac[0]; // 0x00;
  MACAddr[1] = p_mac[1]; // 0x80;
  MACAddr[2] = p_mac[2]; // 0xE1;
  MACAddr[3] = p_mac[3]; // 0x00;
  MACAddr[4] = p_mac[4]; // 0x00;
  MACAddr[5] = p_mac[5]; // 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1536;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

   HAL_ETH_Init(&heth);
#endif
}


void bsp_eth_power_down(void)
{
  uint32_t reg_val;
  if (HAL_ETH_ReadPHYRegister(&heth, 32, 0, &reg_val) != HAL_OK)
  {
    return ;
  }

  reg_val |= (1<<11);

  HAL_ETH_WritePHYRegister(&heth, 32, 0,reg_val );

}