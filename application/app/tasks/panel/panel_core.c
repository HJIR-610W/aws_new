#include <stdint.h>



uint8_t  radd_dirc(uint32_t ang)
{
  uint32_t d_ang;
    uint8_t rr=0;

    d_ang = ang;
    if((d_ang > 0 && d_ang <= 10) || (d_ang > 350))
    {
        rr = 0;
    }
    else
    if(d_ang > 10 && d_ang <= 35) 
    {
        rr = 1;
    }
    else
    if(d_ang > 35 && d_ang <= 60)
    {
        rr = 2;
    }
    else
    if(d_ang > 60 && d_ang <= 80)
    {
        rr = 3;
    }
    else
    if(d_ang > 80 && d_ang <= 100)
    {
        rr = 4;
    }
    else
    if(d_ang > 100 && d_ang <= 125)
    {
        rr = 5;
    }
    else
    if(d_ang > 125 && d_ang <= 150)
    {
        rr = 6;
    }
    else
    if(d_ang > 150 && d_ang <= 170)
    {
        rr = 7;
    }
    else
    if(d_ang > 170 && d_ang <= 190)
    {
        rr = 8;
    }
    else
    if(d_ang > 190 && d_ang <= 215)
    {
        rr = 9;
    }
    else
    if(d_ang > 215 && d_ang <= 240)
    {
        rr = 10;
    }    
    else
    if(d_ang > 240 && d_ang <= 260)
    {
        rr = 11;
    }    
    else
    if(d_ang > 260 && d_ang <= 280)
    {
        rr = 12;
    }
    else
    if(d_ang > 280 && d_ang <= 305)
    {
        rr = 13;
    }
    else
    if(d_ang > 305 && d_ang <= 330)
    {
        rr = 14;
    }
    else
    if(d_ang > 330 && d_ang <= 350)
    {
        rr = 15;
    }

    return(rr);
}    
