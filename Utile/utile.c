

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "utile.h"

int getPinNumber(uint16_t pin) {
  if (pin == 0) {
    return -1; // 입력이 0인 경우, 유효하지 않음
  }

  int position = 0;
  while (pin != 0) {
    if (pin & 1) {
      return position; // 첫 번째로 1인 비트의 위치 반환
    }
    pin >>= 1;
    position++;
  }
  return -1; // 비트가 1인 위치가 없으면 -1 반환
}



void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length) {
    // bit_length만큼의 이진수 문자열 생성
    for (int i = bit_length - 1; i >= 0; i--) {
        binary_str[bit_length - 1 - i] = (hex_value & (1 << i)) ? '1' : '0';
    }
    binary_str[bit_length] = '\0';  // 문자열 종료 문자 추가
}





uint16_t swap_uint16(uint16_t value)
{
  uint16_t ret = 0;

  ret = (value >> 8 &0x00FF);
  ret |= (value << 8);

  return ret;
}


uint16_t  GetWord(uint8_t* lpBuff)
{
    uint16_t shVal;
    
    shVal = ((uint16_t)lpBuff[0] << 8) |(uint16_t)lpBuff[1];

    return(shVal);
}   

void    SetWord(uint8_t *lpBuff, uint16_t shVal)		// Big Endiand으로 취함
{
    lpBuff[0] = (uint8_t)(shVal >> 8);
    lpBuff[1] = (uint8_t)shVal;
}



uint32_t parse_args2(char* str, char* argv[], uint32_t argvCnt)
{
	uint32_t i = 0;
	char* ch = str;

	while (*ch != '\0')
	{
		i++;
		if (i > argvCnt)
		{
			return 0;
		}

		argv[i - 1] = ch;

		while (*ch != ',' && *ch != '\0' && *ch != '\r')
		{
			ch++;
		}
		if (*ch == '\r')
		{
			*ch = '\0';
			break;
		}
		if (*ch != '\0')
		{
			*ch = '\0';
			ch++;
			while (*ch == ':')
			{
				ch++;
			}
		}
	}
	return i;
}



char * h_findnum(char *buff)
{
	char *ptr = NULL;
	int32_t i;
	int32_t cnt = 0;
    int32_t len;

    len = strlen((char *)buff);

	for (i = 0; i < len; i++)
	{
		cnt = 0;
		if (buff[i] == '\"')
		{
			if ((isDigit(buff[i + 1]) == true)|| (buff[i + 1] == '+'))
			{
				ptr = &buff[i + 1];
				cnt++;
				for (i = i + 2; i < len; i++)
				{
					if (isDigit(buff[i]) == true)
					{
						cnt++;
						continue;
					}
					else
					{
						if (buff[i] == '\"')
						{
							buff[i] = 0;

							return ptr;
						}
						else
						{
							ptr = NULL;
							cnt = 0;
							break;
						}
					}
				}
			}
		}
	}

	return ptr;
}



uint32_t parse_args(char* str, char* argv[],uint32_t argvCnt)
{
    uint32_t i = 0;
    char* ch = str;

    while (*ch != '\0')
    {
        i++;
        if (i > argvCnt)
        {
            return 0;
        }

        argv[i - 1] = ch;

        while (*ch != ',' && *ch != '\0' && *ch != '\r')
        {
            if (*ch == ':' && i == 1)
            {
                break;
            }
            else
            {
                ch++;
            }
        }
        if (*ch == '\r')
        {
            *ch = '\0';
            break;
        }
        if (*ch != '\0')
        {
            *ch = '\0';
            ch++;
            while (*ch == ':')
            {
                ch++;
            }
        }
    }
    return i;
}

bool isDigit(uint8_t d)
{
	if ((d >= '0') && (d <= '9'))
		return true;/*digit*/
	else
		return false;/*Not digit*/
}


/**
 * @brief 문자열 복사, 확실리 문자열로 만듦
 * @param det 대상 주소
 * @param detSize 대상 주소 크기
 * @param src 소스
 * @todo 코드가 효율적이 않아서 최적화 코드 작성필요
 */
void strcpy_safe(char* det, size_t detSize, const char* src)
{
    
	detSize--;
	while (detSize &&*src)
	{
		*det++ = *src++;
		detSize--;
	}
	*det = 0;
}

size_t memcpy_safe(uint8_t* des, size_t desLen, uint8_t* src, size_t len)
{

	if(len>desLen)
	{
		len = desLen;
	}
	memcpy(des,src,len);

	return len;
}



uint8_t Convert_Ascii2Hex(uint8_t ascii)
{
	uint8_t hex;

	if ((ascii >= '0') && (ascii <= '9'))
	{
		hex = ascii - '0';
	}
	else if ((ascii >= 'A') && (ascii <= 'F'))
	{
		hex = ascii - 'A'+10;
	}
	return hex;
}

uint32_t Convert_HexAscii2uchar(char* src, uint16_t len, uint8_t * dst)
{
	int i;
	//uint8_t val;

	for (i = 0; i < (len / 2); i++)
	{
		dst[i] = Convert_Ascii2Hex(src[i * 2]) * 16 + Convert_Ascii2Hex(src[i * 2 + 1]);

	}

	return (len / 2);

}



uint8_t Convert_Hex2Ascii(uint8_t hex)
{
	uint8_t ascii=0;
	if( hex <= 9)
	{
		ascii = hex + '0';
	}
	else if ((hex >= 10) && (hex <= 15))
	{
		ascii = hex + 'A' - 10;

	}

	return ascii;
}

uint32_t Convert_ucharHexAscii(uint8_t* src, uint16_t len, char* dst)
{
	unsigned long i;
	uint8_t val;



	for (i = 0; i < len; i++)
	{
		val = src[i];

		dst[i * 2] = Convert_Hex2Ascii((val >> 4) & 0x0F);
		dst[i * 2 + 1] = Convert_Hex2Ascii(val & 0x0F);

	}

	return len * 2;
}

//float ±16,777,216
float recursiveAvg(double pre_avg,float adc, int cnt)
{
  float avg;

  avg = ((cnt - 1) * pre_avg) / cnt + adc / cnt;

  return avg;

}


