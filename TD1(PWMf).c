#include "xc_configuration_bits.h"
#include "plib/adc.h"
#include "plib/timers.h"
#include "plib/delays.h"
#include "math.h"
#include "plib/pwm.h"

#define time360 2000

void move(int angle, unsigned char forward);
void config(void);
void Rmotor(unsigned char power);
void Lmotor(unsigned char power);

int main(void)
{
    config();

    move(0,1); //forward
    move(0,-1); //backward

    move(90,0); //90R (90T)
    move(0,1); //forward
    move(0,-1); //backward

    move(-180,0); //180L (270T)
    move(0,1); //forward
    move(0,-1); //backward

    move(-90,0); //90L (180T)
    move(0,1); //forward
    move(0,-1); //backward

    move(180,0); //180R (0T)

    move(90,1); //curve right
    move(90,-1); //back

    move(90,1); //curve right
    move(90,-1); //back

    move(-90,1); //curve left
    move(-90,-1); //back

    ClosePWM1();
    ClosePWM2();
}

void config(void)
{
    PORTAbits.RA0 = 1; //bipolar setting - wire in hardware to Jp1 1
    PORTAbits.RA1 = 1; //bipolar setting - wire in hardware to Jp1 4
    //direction omitted as only relevant for unipolar
    OpenTimer2(TIMER_INT_OFF & T2_PS_1_16 & T2_POST_1_1);
}

void move(int angle, unsigned char forward)
{
    int exec_time_ms = ((double)abs(angle)/360)*time360;

    unsigned char pr = 0;
    unsigned char pl = 0;

    if(forward == 1)
    {
        if(angle > 0)
        {
            pr = 100;
            pl = 75;
        }
        else if (angle < 0)
        {
            pr = 75;
            pl = 100;
        }
        else
        {
            pr = 100;
            pl = 100;
        }
    }
    else if (forward == 0)
    {
        if(angle > 0)
        {
            pr = 75;
            pl = 50;
        }
        else if (angle < 0)
        {
            pr = 50;
            pl = 75;
        }
        else
        {
            pr = 50;
            pl = 50;
        }
    }
    else if (forward == -1)
    {
        if(angle > 0)
        {
            pr = 25;
            pl = 0;
        }
        else if (angle < 0)
        {
            pr = 0;
            pl = 25;
        }
        else
        {
            pr = 0;
            pl = 0;
        }
    }

    Rmotor(pr);
    Lmotor(pl);

    for(int i = 0; i < exec_time_ms; i++)
    {
        Delay10TCYx(250);
    }
}


void Rmotor(unsigned char power)
{
    OpenPWM1(0x01); //P = (0x01+1)*4*Tosc*TMR2 <- TBD use at least 5kHz

    SetDCPWM1(power);
}

void Lmotor(unsigned char power)
{
    OpenPWM2(0x01); //P = (0x01+1)*4*Tosc*TMR2 <- TBD

    SetDCPWM2(power);

    //+ set the PicPWM conected to PWMpin3
}
