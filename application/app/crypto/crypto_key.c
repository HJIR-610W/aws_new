
#include "crypto_key.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h> 
#include <time.h>   

// 4x4 키 테이블
#define TABLE_SIZE 4



//미리 비빌번호를 table로 만들어서 놓고 인덱스로 관리한다.
//이렇게 아하면 펌웨어 읽어보면 상수 영역에 비밀번호가 노출되기 싶다.
char key_table[TABLE_SIZE*TABLE_SIZE]={'9','2','1','4',
                                       '6','4','5','1',
                                       '3','0','5','1',
                                       '3','4','0','6'};

uint8_t key_index[4] = {2, 4, 9, 14}; 


// 비밀번호 길이 상수


/**
 * @brief key_table에서 key_index를 참조하여 비밀번호를 복원합니다.
 * @param p_out_key 복원된 비밀번호를 저장할 출력 버퍼
 */
void read_password(char* p_out_key)
{
    int i;
    
    if (p_out_key == NULL) {

        return;
    }

    for (i = 0; i < PASSWORD_LENGTH; ++i) 
    {
        p_out_key[i] = key_table[key_index[i]];
    }
    
    p_out_key[PASSWORD_LENGTH] = '\0';
}

