
#include <stdint.h>
#include <stdio.h>

#include "pcb_define.h"
#include "cmsis_os.h"
#include "driver_uart.h"
#include "ymodem.h"
#include "io.h"

#define DEBUG_PORT    eCOM5_DEBUG    

#define IS_CAP_LETTER(c)    (((c) >= 'A') && ((c) <= 'F'))
#define IS_LC_LETTER(c)     (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c)            (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c)       (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c)       IS_09(c)
#define CONVERTDEC(c)       (c - '0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - 'A'+10) : ((c) - 'a'+10))
#define CONVERTHEX(c)       (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))


#define CRC16_F       /* activate the CRC16 integrity */

#define FW_FLASH_SIZE 1024
#define USER_FLASH_SIZE  FW_FLASH_SIZE

#define APPLICATION_ADDRESS 0

#define Serial_PutByte(x)  do{\
                                uint8_t byte;\
                                byte = x;\
                                debug_send(&byte,1);\
                            }while(0)

#define RECVDATA_SIZE 1

enum 
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR
};


 uint32_t flashdestination;

/* @note ATTENTION - please keep this variable 32bit alligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint8_t aFileName[FILE_NAME_LENGTH];
uint8_t aRecvData[RECVDATA_SIZE];

static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);



void Int2Str(uint8_t *p_str, uint32_t intnum)
{
  uint32_t i, divider = 1000000000, pos = 0, status = 0;

  for (i = 0; i < 10; i++)
  {
    p_str[pos++] = (intnum / divider) + 48;

    intnum = intnum % divider;
    divider /= 10;
    if ((p_str[pos-1] == '0') & (status == 0))
    {
      pos = 0;
    }
    else
    {
      status++;
    }
  }
}


/**
  * @brief  Convert a string to an integer
  * @param  p_inputstr: The string to be converted
  * @param  p_intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X')))
  {
    i = 2;
    while ( ( i < 11 ) && ( p_inputstr[i] != '\0' ) )
    {
      if (ISVALIDHEX(p_inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(p_inputstr[i]);
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }

    /* valid result */
    if (p_inputstr[i] == '\0')
    {
      *p_intnum = val;
      res = 1;
    }
  }
  else /* max 10-digit decimal input */
  {
    while ( ( i < 11 ) && ( res != 1 ) )
    {
      if (p_inputstr[i] == '\0')
      {
        *p_intnum = val;
        /* return 1 */
        res = 1;
      }
      else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0))
      {
        val = val << 10;
        *p_intnum = val;
        res = 1;
      }
      else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0))
      {
        val = val << 20;
        *p_intnum = val;
        res = 1;
      }
      else if (ISVALIDDEC(p_inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(p_inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }
  }

  return res;
}


uint32_t FLASH_If_Erase(uint32_t StartSector)
{

  
  return (0);
}


uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data ,uint32_t DataLength)
{


  return (0);
}


uint32_t write_ramFile(uint8_t * address,uint8_t *data,uint32_t dataLen)
{
    uint8_t *ptr;

    ptr = address;


    if(ptr > &aRecvData[RECVDATA_SIZE-1])
    {
        return 0;
    }


    while(dataLen)
    {
        

        *ptr++ = *data++;
        dataLen--;


    }

    return 0;
}

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
 HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
    uint8_t char1;
    uint16_t len;
    uint32_t crc;
    uint32_t packet_size = 0;
    HAL_StatusTypeDef status=HAL_ERROR;

    *p_length = 0;

    if(debug_recv((char *)&char1,1,timeout))
    {
        switch (char1)
        {
            case SOH:
                packet_size = PACKET_SIZE;
                break;
            case STX:
                packet_size = PACKET_1K_SIZE;
                break;
            case EOT:
                status = (HAL_StatusTypeDef)4;//颇老场
                break;
            case CA:
                if(debug_recv((char *)&char1,1,timeout)&&(char1==CA))
                {
                    packet_size = 2;
                    status = (HAL_StatusTypeDef)5;// 俊矾 
                }
                else
                {
                    status = HAL_ERROR;
                }
                break;
            case ABORT1:
            case ABORT2:
                status = HAL_BUSY;
                break;
            default:
                status = HAL_ERROR;
                break;
        }


    *p_data = char1;

    if (packet_size >= PACKET_SIZE )
    {
        status = HAL_ERROR;
        len = debug_recv((char *)&p_data[PACKET_NUMBER_INDEX],packet_size + PACKET_OVERHEAD_SIZE,timeout);
       // printf("CNT %d,len %d\r\n",p_data[PACKET_NUMBER_INDEX],len);
        if(len)
        {
            status = HAL_OK;
        }

      /* Simple packet sanity check */
        if (status == HAL_OK )
        {
            if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
            {
                packet_size = 0;
                status = HAL_ERROR;
                //printf("err %X %X\r\n",p_data[PACKET_NUMBER_INDEX],p_data[PACKET_CNUMBER_INDEX]);
            }
            else
            {
                /* Check packet CRC */
                crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
                crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
               // printf("crc %X\r\n",crc);
                if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
                {
                    packet_size = 0;
                    status = HAL_ERROR;
                    //printf("crc fail %X\r\n",crc);
                }
            }
        }
      else
      {
        packet_size = 0;
      }
    }
  }
  *p_length = packet_size;
  return status;
}



/**
  * @brief  Prepare the first block
  * @param  p_data:  output buffer
  * @param  p_file_name: name of the file to be sent
  * @param  length: length of the file to be sent in bytes
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
  uint32_t i;
  uint32_t j = 0;
  uint8_t astring[10];

  /* first 3 bytes are constant */
  p_data[PACKET_START_INDEX] = SOH;
  p_data[PACKET_NUMBER_INDEX] = 0x00;
  p_data[PACKET_CNUMBER_INDEX] = 0xff;

  /* Filename written */
  for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
  }

  p_data[i + PACKET_DATA_INDEX] = 0x00;

  /* file size written */
  Int2Str (astring, length);
  i = i + PACKET_DATA_INDEX + 1;
  while (astring[j] != '\0')
  {
    p_data[i++] = astring[j++];
  }

  /* padding with zeros */
  for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
  {
    p_data[j] = 0;
  }
}

/**
  * @brief  Prepare the data packet
  * @param  p_source: pointer to the data to be sent
  * @param  p_packet: pointer to the output buffer
  * @param  pkt_nr: number of the packet
  * @param  size_blk: length of the block to be sent in bytes
  * @retval None
  */
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
  uint8_t *p_record;
  uint32_t i;
  uint32_t size;
  uint32_t packet_size;

  /* Make first three packet */
  packet_size = size_blk >= PACKET_1K_SIZE ? (uint32_t)PACKET_1K_SIZE : (uint32_t)PACKET_SIZE;
  size = size_blk < packet_size ? size_blk : packet_size;
  if (packet_size == PACKET_1K_SIZE)
  {
    p_packet[PACKET_START_INDEX] = STX;
  }
  else
  {
    p_packet[PACKET_START_INDEX] = SOH;
  }
  p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
  p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
  p_record = p_source;

  /* Filename packet has valid data */
  for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX;i++)
  {
    p_packet[i] = *p_record++;
  }
  if ( size  <= packet_size)
  {
    for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
    {
      p_packet[i] = 0x1A; /* EOF (0x1A) or 0x00 */
    }
  }
}

/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }
  
  while(!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = p_data+size;

  while(p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);
 
  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc&0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end )
  {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}





/**
  * @brief  Transmit a file using the ymodem protocol
  * @param  p_buf: Address of the first byte
  * @param  p_file_name: Name of the file sent
  * @param  file_size: Size of the transmission
  * @retval COM_StatusTypeDef result of the communication
  */
COM_StatusTypeDef Ymodem_Transmit (uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
  uint32_t errors = 0, ack_recpt = 0;
  uint32_t size = 0;
  uint32_t pkt_size;
  uint8_t *p_buf_int;
  COM_StatusTypeDef result = COM_OK;
  uint32_t blk_number = 1;
  uint8_t a_rx_ctrl[2];
  uint8_t i;
#ifdef CRC16_F    
  uint32_t temp_crc;
#else /* CRC16_F */   
  uint8_t temp_chksum;
#endif /* CRC16_F */  

  /* Prepare first block - header */
  PrepareIntialPacket(aPacketData, p_file_name, file_size);

  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    /* Send Packet */
  //  HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);
       debug_send(&aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE);
    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
    //if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    if(debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT))
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        //if ((HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
    if(debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
         // __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
         // uart_flush(DEBUG_PORT);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }
    if (errors >= MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  p_buf_int = p_buf;
  size = file_size;

  /* Here 1024 bytes length is used to send the packets */
  while ((size) && (result == COM_OK ))
  {
    /* Prepare next packet */
    PreparePacket(p_buf_int, aPacketData, blk_number, size);
    ack_recpt = 0;
    a_rx_ctrl[0] = 0;
    errors = 0;

    /* Resend packet if NAK for few times else end of communication */
    while (( !ack_recpt ) && ( result == COM_OK ))
    {
      /* Send next packet */
      if (size >= PACKET_1K_SIZE)
      {
        pkt_size = PACKET_1K_SIZE;
      }
      else
      {
        pkt_size = PACKET_SIZE;
      }

    //  HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE, NAK_TIMEOUT);
             debug_send( &aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE);
      /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
      temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_crc >> 8);
      Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
      temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_chksum);
#endif /* CRC16_F */
      
      /* Wait for Ack */
     // if ((HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == ACK))
      if(debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT)&& (a_rx_ctrl[0] == ACK))
      {
        ack_recpt = 1;
        if (size > pkt_size)
        {
          p_buf_int += pkt_size;
          size -= pkt_size;
          if (blk_number == (USER_FLASH_SIZE / PACKET_1K_SIZE))
          {
            result = COM_LIMIT; /* boundary error */
          }
          else
          {
            blk_number++;
          }
        }
        else
        {
          p_buf_int += pkt_size;
          size = 0;
        }
      }
      else
      {
        errors++;
      }

      /* Resend packet if NAK  for a count of 10 else end of communication */
      if (errors >= MAX_ERRORS)
      {
        result = COM_ERROR;
      }
    }
  }

  /* Sending End Of Transmission char */
  ack_recpt = 0;
  a_rx_ctrl[0] = 0x00;
  errors = 0;
  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    Serial_PutByte(EOT);

    /* Wait for Ack */
    //if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    if(debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT))
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if (debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
         // __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
                   //uart_flush(DEBUG_PORT);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }

    if (errors >=  MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  /* Empty packet sent - some terminal emulators need this to close session */
  if ( result == COM_OK )
  {
    /* Preparing an empty packet */
    aPacketData[PACKET_START_INDEX] = SOH;
    aPacketData[PACKET_NUMBER_INDEX] = 0;
    aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
    for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
    {
      aPacketData [i] = 0x00;
    }

    /* Send Packet */
   // HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);
       debug_send( &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE);
    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
  //  if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    if(debug_recv((char *)&a_rx_ctrl[0], 1, NAK_TIMEOUT))
    {
      if (a_rx_ctrl[0] == CA)
      {
          HAL_Delay( 2 );
      //    __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
          //uart_flush(DEBUG_PORT);
          result = COM_ABORT;
      }
    }
  }

  return result; /* file transmitted successfully */
}


#define EXT_FLSH_USE 1




int32_t download_file(int32_t (*write_file)(char *path,uint32_t offset,uint8_t *data,uint32_t dataLen),
                        char *path,uint32_t offset,uint32_t *len,uint32_t limit)
{
    uint32_t i;
    uint32_t packet_length;
    uint32_t session_done = 0;
    uint32_t file_done;
    uint32_t errors = 0;
    uint32_t session_begin = 0;
    uint32_t  filesize;
    uint8_t *file_ptr;
    uint8_t file_size[FILE_SIZE_LENGTH],  packets_received;
    COM_StatusTypeDef result = COM_OK;
    int32_t err=1;
    uint32_t receivedSize=0;



    flashdestination = 0;


   // uart_flush(DEBUG_PORT);
     *len = 0;
    while ((session_done == 0) && (result == COM_OK))
    {
        packets_received = 0;
        file_done = 0;
        while ((file_done == 0) && (result == COM_OK))
        {
            switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
            {
                case 5:
                return 5;
                break;
                
                case 4:// 颇老场
                err = 0;
                                                        debug_putch(CA);
                                        debug_putch(CA);
                                        
                return 0;
                break;
                case HAL_OK:
                    errors = 0;
                    switch(packet_length)
                    {
                        case 2:
                            /* Abort by sender */
                            debug_putch(ACK);
                            result = COM_ABORT;
                        break;
                        case 0:
                            /* End of transmission */
                            debug_putch(ACK);
                            file_done = 1;
                        break;
                        default:
                            if (aPacketData[PACKET_NUMBER_INDEX] != packets_received)
                            {
                                debug_putch(NAK);
                            }
                            else
                            {
                                if (packets_received == 0)
                                {
                                    /* File name packet */
                                    if (aPacketData[PACKET_DATA_INDEX] != 0)
                                    {
                                        /* File name extraction */
                                        i = 0;
                                        file_ptr = aPacketData + PACKET_DATA_INDEX;
                                         while ( (*file_ptr != 0) && (i < (FILE_NAME_LENGTH-1)))
                                        {
                                        aFileName[i++] = *file_ptr++;
                                        }

                                        /* File size extraction */
                                        aFileName[i++] = '\0';
                                        i = 0;
                                        file_ptr ++;
                                        while((*file_ptr != ' ') && (i < (FILE_SIZE_LENGTH - 1)))
                                        {
                                            file_size[i++] = *file_ptr++;
                                        }
                                        file_size[i] = '\0';
                                        Str2Int(file_size, &filesize);


                                        if (filesize > limit )
                                        {
                                        debug_putch(CA);
                                        debug_putch(CA);
                                        result = COM_LIMIT;
                                        err = 2;
                                        }
                                        debug_putch(ACK);
                                        debug_putch(CRC16);
                                    }
                                    /* File header packet is empty, end session */
                                    else
                                    {
                                        debug_putch(ACK);
                                        file_done = 1;
                                        session_done = 1;
                                        break;
                                    }
                                }
                                else /* Data packet */
                                {
                                    receivedSize += packet_length;

                                       *len = receivedSize;
                               
                            
                                   
                                    err =  write_file(path,offset + flashdestination,&aPacketData[PACKET_DATA_INDEX],packet_length);
                                    
                                    
                                    if(err)
                                    {
                                        debug_putch(CA);
                                        debug_putch(CA);
                                        result = COM_DATA;
                                        return 1;
                                    }
                                    else 
                                    {
                                     
                                              
                                        flashdestination += packet_length;


                                        debug_putch(ACK);
                                    }
                                }
                                packets_received ++;
                                session_begin = 1;
                            }
                     break;
                    }//switch end HAL_OK
                    break;
                case HAL_BUSY: /* Abort actually */
                    debug_putch(CA);
                    debug_putch(CA);
                    result = COM_ABORT;
                    return 1;
                    break;
                default:
                    if (session_begin > 0)
                    {
                        errors ++;
                        osDelay(10);
                    }
                    if(errors > MAX_ERRORS)
                    {
                        osDelay(10);
                        /* Abort communication */
                        debug_putch(CA);
                        debug_putch(CA);

                        return  1;
                    }
                    else
                    {
                        osDelay(10);
                        debug_putch(CRC16); /* Ask for a packet */
                    }
                    break;
                }
            }
        }

  return 0;
}