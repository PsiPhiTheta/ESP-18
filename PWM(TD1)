#include "xc_configuration_bits.h"
#include "plib/adc.h"
#include "plib/timers.h"
#include "plib/delays.h"
#include "math.h"
#include "plib/pwm.h"

#define time360 2000

void move (int angle, unsigned char forward);
void configure_MDB(void);

int main(void)
{
    configure_MDB();

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
}

void configure_MDB(void)
{
    PORTAbits.RA0 = 1; //bipolar setting
    PORTAbits.RA1 = 1; //bipolar setting
    //direction omitted as only relevant for unipolar
}

void move (int angle, unsigned char forward)
{
    int exec_time_ms = ((double)abs(angle)/360)*time360;

    unsigned char pr = 0;
    unsigned char pl = 0;

    if(forward == 1)
    {
        if(angle > 0)
        {
            pr = 255;
            pl = 192;
        }
        else if (angle < 0)
        {
            pr = 192;
            pl = 255;
        }
        else
        {
            pr = 255;
            pl = 255;
        }
    }
    else if (forward == 0)
    {
        if(angle > 0)
        {
            pr = 192;
            pl = 128;
        }
        else if (angle < 0)
        {
            pr = 128;
            pl = 192;
        }
        else
        {
            pr = 128;
            pl = 128;
        }
    }
    else if (forward == -1)
    {
        if(angle > 0)
        {
            pr = 64;
            pl = 0;
        }
        else if (angle < 0)
        {
            pr = 0;
            pl = 64;
        }
        else
        {
            pr = 0;
            pl = 0;
        }
    }

    PORTCbits.RC2 = pl;
    PORTBbits.RB3 = pr;

    for(int i = 0; i < exec_time_ms; i++)
    {
        Delay10TCYx(250);
    }
}
