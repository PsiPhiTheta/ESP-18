//0. Code information
    //Authors: Thomas Hollis, Charles Shelbourne
    //Project: ESP-18
    //Year: 2017
    //Version: 1.0

//1. File inclusions required
    #include "xc_configuration_bits.h"
    #include "adc.h"
    #include "timers.h"
    #include "delays.h"
    #include "math.h"
    #include "pwm.h"
    #include "capture.h"

//2. Function prototypes

//Configuration functions
    void config_PWM(void);
    void config_LS(void);
    void config_PS(void);

//Motor functions
    void move(int left);
    void Rmotor(int power);
    void Lmotor(int power);
    void stop();

//Line sensor functions
    void LEDarray_on(void);
    void LEDarray_off(void);
    void LSarray_read(void);
    void LEDarray_write(unsigned char x);
    unsigned char LEDarray_breakdetected(void);

//Proximity sensor functions
    void interrupt isr(void);
    void enable_global_interrupts(void);

//Speed encoder functions
    //none required yet

//3. Global variables
    int x = 0;
    int time180 = 1000;

    volatile char y=0,s=0;
    volatile unsigned int logic_high =0;
    
    unsigned char LS_val[6] = {0, 0, 0, 0, 0, 0};

//4. Main Line Code
    int main(void)
    {
        config_LS();
        config_PS();
        config_PWM();
        //enable_global_interrupts();
        OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_12_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 0);

        while (1)
        {
            LSarray_read();

            int i = LS_val[0]^LS_val[1];
            int k = LS_val[2]^LS_val[4];
            
            unsigned char right = 2*LS_val[0] + 1*i*LS_val[1];
            unsigned char left = 2*LS_val[4] + 1*k*LS_val[2];
            
         
            if(left>right)
            {
                move(left);
            }
            else if(left<right)
            {
                move(-right);
            }
            else if (right == 0 && left == 0)
            {
                stop();
            }
            
            
        }

    }

//5. Functions
//Configuration functions
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

    void config_PS(void)
    {
        OpenTimer3(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_1 & T1_OSC1EN_ON & T1_SYNC_EXT_OFF);
        OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_1);
        WriteTimer0(12536);
        OpenCapture3(CAPTURE_INT_ON & C3_EVERY_RISE_EDGE);

        TRISB =0x00;
        TRISC = 0x00;
    }

//Motor functions
    
    void stop()
    {
        PORTHbits.RH3 = 0;
    }
    
    void move(int error)
    {
        PORTHbits.RH3 = 1;
        Lmotor(700+50*error);
        Rmotor(700+50*(-error));
        Delay10KTCYx(225);
        PORTHbits.RH3 = 0;  
    }
    
    void turn180()
    {
        PORTHbits.RH3 = 1;
        Rmotor(300);
        Lmotor(700);
        for(int i = 0; i < time180; i++)
        {
            Delay10TCYx(200);
            Delay10TCYx(200);
        }
        PORTHbits.RH3 = 0;
    }
    
    void Rmotor(int power)
    {
        SetDCPWM4(power);
    }
    void Lmotor(int power)
    {
        SetDCPWM5(power);
    }

//Line sensor functions
    void LEDarray_on(void)
    {
        LATA = 0b00111111;
    }
    void LEDarray_off(void)
    {
        LATA = 0b00000000;
    }
    void LSarray_read(void)
    {
        int value = 0;

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
            unsigned char temp = i;
            if(value > 700)
                LS_val[i] = 1;
            else
                LS_val[i] = 0;
        }
    }
    void LEDarray_write(unsigned char x)
    {
        LATA = x;
    }
    unsigned char LEDarray_breakdetected(void)
    {
        unsigned char breakdetected = 0;
        breakdetected = 1;
        return breakdetected;
    }

    //Proximity sensor functions
    void enable_global_interrupts(void)
    {
        INTCONbits.GIE = 1;
        INTCONbits.PEIE = 1;
    }
    void interrupt isr(void)
    {
        if(INTCONbits.TMR0IF) //Proximity trigger signal
        {
            INTCONbits.TMR0IF =0;
            s = s^1;
            LATBbits.LATB0 = s;   //J2 13
            WriteTimer0(40563);

        }
        if(PIR3bits.CCP3IF == 1)
        {
            PIR3bits.CCP3IF = 0;  //CCP4 interrupt bit zeroed
            y= y^1;              //switched between 1 and 0

            if(y==1)
            {
                CCP3CON = 4;  //configure CCP 4 to interrupt on falling edge
                WriteTimer3(0);  //refresh timer3
            }
            else
            {
                logic_high = ReadTimer3();
                CCP3CON = 5;     //configure to interrupt on rising edge
                if(logic_high<2040 && logic_high >= 1360)
                {    //1360 ticks of clock before LED's turn on relates to 544uS that echo signal is high => aprox 11.3cm
                    turn180();     // uS/48 = distance
                }
                else if(logic_high<1360 && logic_high >= 680)
                {    //1360 ticks of clock before LED's turn on relates to 544uS that echo signal is high => aprox 11.3cm
                    turn180();     // uS/48 = distance
                }  
                else if (logic_high < 680)
                {
                    turn180(); 
                }
                else
                {
                    LATC=0x00;
                }
            }

        }
    }
    //2e. Speed encoder functions
        //none required yet
