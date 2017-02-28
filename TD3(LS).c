//0. Code information
    //Authors: Thomas Hollis, Charles Shelbourne
    //Project: ESP-18
    //Year: 2017
    //Version: 2.1

//1. File inclusions required
    #include "xc_configuration_bits.h"
    #include "plib/adc.h"
    #include "plib/timers.h"
    #include "plib/delays.h"
    #include "math.h"
    #include "plib/pwm.h"

//2. Function prototypes
    //2a. Configuration functions
        void config_PWM(void);
        void config_LS(void);

    //2b. Motor functions
        void move(int angle, int forward);
        void Rmotor(int power);
        void Lmotor(int power);

    //2c. Line sensor functions
        void LEDarray_on(void);
        void LEDarray_off(void);
        unsigned char LSarray_read(void);
        void LEDarray_write(unsigned char x);
        unsigned char LEDarray_breakdetected(void);

    //2d. Proximity sensor functions
        //none required yet

    //2e. Speed encoder functions
        //none required yet

//3. Global variables
    int x = 0;
    int time360 = 2000;

//4. Main Line Code (MLC)
    int main(void)
        {
            config_LS();

            LEDarray_on();
            Delay10KTCYx(250);
            LEDarray_off();
            Delay10KTCYx(250);

            while (LEDarray_breakdetected() == 0)
            {
                LEDarray_off();
            }

            LEDarray_on();
            Delay10KTCYx(25);
            LEDarray_off();
            Delay10KTCYx(25);
            LEDarray_on();
            Delay10KTCYx(25);
            LEDarray_off();
            Delay10KTCYx(25);

            OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_12_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 0);

            while (1)
            {
                LEDarray_write(LSarray_read());
                //Delay1KTCYx(25);
            }

        }

//5. Functions
    //2a. Configuration functions
        void config_PWM(void)
            {
                //pwm output
                TRISGbits.RG3 = 0;
                TRISGbits.RG4 = 0;

                TRISH = 0b10010100;

                //enable bit
                PORTHbits.RH3 = 0;

                //unipolar setting
                PORTHbits.RH0 = 1;
                PORTHbits.RH1 = 1;

                //direction bits
                PORTHbits.RH5 = 0;
                PORTHbits.RH6 = 0;

                //timer configuration
                OpenTimer2(TIMER_INT_OFF & T2_PS_1_1 & T2_POST_1_1);

                //OpenPWM2
                OpenPWM4(252);
                OpenPWM5(252);
            }
        void config_LS(void)
            {    
                ADCON1 = 0x00;
                TRISA = 0b00000000;
            }

    //2b. Motor functions
        void move(int angle, int forward)
            {
                int exec_time_ms = ((double)sqrt(angle*angle)/360.0)*time360;
                if(forward == 3)
                {
                    PORTHbits.RH3 = 1;
                    Rmotor(700);
                    Lmotor(800);
                    Delay10KTCYx(225);
                    PORTHbits.RH3 = 0;
                }
                if(forward == 2)
                {
                    PORTHbits.RH3 = 1;
                    Rmotor(700);
                    Lmotor(700);
                    Delay10KTCYx(125);
                    Delay10KTCYx(125);
                    PORTHbits.RH3 = 0;
                }
                if(forward == 1)
                {
                    PORTHbits.RH3 = 1;
                    Rmotor(625);
                    Lmotor(625);
                    Delay10KTCYx(125);
                    Delay10KTCYx(125);
                    PORTHbits.RH3 = 0;
                }
                else if(forward == -1)
                {
                    PORTHbits.RH3 = 1;
                    Rmotor(375);
                    Lmotor(375);
                    Delay10KTCYx(125);
                    Delay10KTCYx(125);
                    PORTHbits.RH3 = 0;
                }
                else if(forward == 0)
                {
                    if(angle > 0)
                    {
                        PORTHbits.RH3 = 1;
                        Rmotor(700);
                        Lmotor(300);
                        for(int i = 0; i < exec_time_ms; i++)
                        {
                            Delay10TCYx(200);
                            Delay10TCYx(200);
                        }
                        PORTHbits.RH3 = 0;
                    }
                    else
                    {
                        PORTHbits.RH3 = 1;
                        Rmotor(300);
                        Lmotor(700);
                        for(int i = 0; i < exec_time_ms; i++)
                        {
                            Delay10TCYx(200);
                            Delay10TCYx(200);
                        }
                        PORTHbits.RH3 = 0;
                    }
                }
            }
        void Rmotor(int power)
            {
                SetDCPWM4(power);
            }
        void Lmotor(int power)
            {
                SetDCPWM5(power);
            }

    //2c. Line sensor functions
        void LEDarray_on(void)
            {
                LATA = 0b00111111;
            }
        void LEDarray_off(void)
            {
                LATA = 0b00000000;
            }
        unsigned char LSarray_read(void)
            {
                unsigned char LS_val[6] = {0, 0, 0, 0, 0, 0};
                unsigned char LS_array = 0;
                int value = 0;
                int checking = 0;
                
                for(int i = 0; i < 6; i++)
                {
                    value = 0;
                    switch(i)
                    {
                        case 0: SetChanADC(ADC_CH5);
                        break;
                        case 1: SetChanADC(ADC_CH6);
                        break;
                        case 2: SetChanADC(ADC_CH7);
                        break;
                        case 3: SetChanADC(ADC_CH8);
                        break;
                        case 4: SetChanADC(ADC_CH9);
                        break;
                        case 5: SetChanADC(ADC_CH10);
                        break;
                        default:break;
                    }
                    
                    ConvertADC();
                    while(BusyADC());
                    value = ReadADC();
                    
                    if(value > 700)
                        LS_val[i] = 1;
                    else
                        LS_val[i] = 0;
                    
                    checking = LS_val [i];
                    LS_array += LS_val[i]*pow(2,i);
                }

                return LS_array;

            }
        void LEDarray_write(unsigned char x)
            {
                LATA = x;
            }
        unsigned char LEDarray_breakdetected(void)
            {
                unsigned char breakdetected = 0;
                //Tom to Charlie: write a function that detects a small drop in voltage (comparator may be needed, I may join in on this one too - we might also bullshit our way through it)
                breakdetected = 1;
                return breakdetected;
            }

    //2d. Proximity sensor functions
        //none required yet

    //2e. Speed encoder functions
        //none required yet
