//Authors: Thomas Hollis, Charles Shelbourne
//Version: 7.0

#include "xc_config_settings.h"
#include "delays.h"
#include "timers.h"
#include "math.h"
#include "capture.h"
#include "pwm.h"
#include "adc.h"

#define WHITE_LINE 800
#define WALL_DISTANCE 4000
#define BASE_SPEED 750
#define MAX_SPEED 820
#define BREAK_SPEED 350
#define Kp 9
#define Ki 0
#define Kd 4

void interrupt isr(void);
void setup_PWM(void);
void setup_line_sensors(void);
void setup_proximity_sensor(void);
void setup_global_interrupts(void);
void read_line_sensors(void);
int calculate_proportional_error(void);
int calculate_PID_error(int error);
void move_algorithm(int PID_error);
void move_180 (void);
void move_stop(void);
void motor_right(int power);
void motor_left(int power);

unsigned char line_sensor[6] = {0, 0, 0, 0, 0, 0}; //stores digital 1 or 0 for each line sensor

int P = 0; //proportional term
int I = 0; //integral term
int D = 0; //derivative term

int previous_error = 0; //stores value of previous error for Kd
int counter = 0; //sets the refresh rate of Kd

volatile char x=0; //temporary variable for ISR
volatile char y=0; //temporary variable for ISR
volatile unsigned int distance = 0; //ISR variable to store distance of proximity sensor to a wall

int main(void)
{
    setup_line_sensors();
    setup_proximity_sensor();
    setup_PWM();
    setup_global_interrupts();

    while (1)
    {
        read_line_sensors(); //updates line_sensors array with new values of line sensors
        
        int sensor_array_val = 0; //contains all values of sensor array
        for(int i = 0; i < 6; i++) //reads all sensor values and writes into variable
        {
            sensor_array_val = sensor_array_val + line_sensor[i]; 
        }
        if(sensor_array_val == 0) //if all sensors on black
        {
            Delay10KTCYx(70); //wait for momentum to stop
            if(sensor_array_val == 0) //if all sensors still on black
            {
                move_stop(); //stop
                Delay10KTCYx(20); //waits for stop to finish executing
                INTCONbits.GIE = 0; //disables interrupts
                INTCONbits.PEIE = 0; //disables interrupts
                PORTHbits.RH3 = 0; //disables enable bit
                while(1); //stops forever
            }
        }
        else
        {
            move_algorithm(calculate_PID_error(calculate_proportional_error())); //move according to PID algorithm
        }
    }
}

void interrupt isr(void)
{
    if(INTCONbits.TMR0IF) //Proximity trigger signal
    {
        INTCONbits.TMR0IF =0;
        x = x^1;
        LATBbits.LATB0 = x;   //J2 13
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
            distance = ReadTimer3();
            CCP3CON = 5;     //configure to interrupt on rising edge
            if(distance < WALL_DISTANCE)
            {    //1360 ticks of clock before LED's turn on relates to 544uS that echo signal is high => aprox 11.3cm
                move_180();     // uS/48 = distance
            }
        }
    }
}    

void setup_PWM(void)
{
    TRISGbits.RG3 = 0; //setup IO
    TRISGbits.RG4 = 0; //setup IO
    TRISH = 0b10010100; //setup IO

    PORTHbits.RH3 = 0; //enable bit

    PORTHbits.RH0 = 1; //bipolar
    PORTHbits.RH1 = 1; //bipolar

    PORTHbits.RH5 = 0; //direction bit
    PORTHbits.RH6 = 0; //direction bit

    OpenTimer2(TIMER_INT_OFF & T2_PS_1_1 & T2_POST_1_1); //timer config

    OpenPWM4(252); //setup PWM
    OpenPWM5(252); //setup PWM
}

void setup_line_sensors(void)
{
    ADCON1 = 0x00; //setup analogue/digital
    TRISA = 0b00000000; //setup IO
    OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_12_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 0); //configures ADC
}

void setup_proximity_sensor(void)
{
    OpenTimer3(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_1 & T1_OSC1EN_ON & T1_SYNC_EXT_OFF); //setup timer
    OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_1); //setup timer
    WriteTimer0(12536); //setup timer offset

    OpenCapture3(CAPTURE_INT_ON & C3_EVERY_RISE_EDGE); //setup Capture

    TRISB =0x00; //setup IO
    TRISC = 0x00; //setup IO
}
    
void setup_global_interrupts(void)
{
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void read_line_sensors(void)
{
    for(int i = 0; i < 6; i++)
    {
        int value = 0;
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

        if(value > WHITE_LINE)
        {
            line_sensor[i] = 1; //assign binary 1/true/on
        }
        else
        {
            line_sensor[i] = 0; //assign binary 0/false/off
        }
    }
}
    
int calculate_proportional_error(void)
{
    int sensors_detected = line_sensor[0] + line_sensor[1] + line_sensor[2] + line_sensor[3] + line_sensor[4] + line_sensor[5];
    
    switch (sensors_detected) //returns different error if sensor collisions occur
    {
        case 1:  //no sensor collisions
            return (80*line_sensor[0] + 20*line_sensor[1] + 1*line_sensor[2] + -1*line_sensor[3] + -20*line_sensor[4] + -80*line_sensor[5]); //coefficients set by trial and error
            
        case 2: //2 sensor collisions 
            return (46*line_sensor[0] + 25*line_sensor[1] + 0*line_sensor[2] + 0*line_sensor[3] + -25*line_sensor[4] + -46*line_sensor[5]); //similar to previously but middle is ignored and outer is reinforced
        
        case 3: //3 sensor collisions
            return (24*line_sensor[0] + 20*line_sensor[1] + 10*line_sensor[2] + -10*line_sensor[3] + -20*line_sensor[4] + -24*line_sensor[5]); //similar to previously but less strong since 3 triggered
        
        case 4: //4 sensor collisions
            return (15*line_sensor[0] + 0*line_sensor[1] + 0*line_sensor[2] + 0*line_sensor[3] + 0*line_sensor[4] + -15*line_sensor[5]);
            
        default:
            return 0;
    }
}

int calculate_PID_error(int error)
{
    counter++;

    P = Kp*error;
    I = Ki*0; //not yet used
    D = Kd*(error - previous_error);

    if(counter == 10) //checks if its time to update D
    {
        previous_error = error; //updates D
        counter = 0; //resets counter
    }

    return (P + I + D);
}
    
void move_algorithm(int PID_error)
{
    PORTHbits.RH3 = 1; //enables enable bit

    if(PID_error > MAX_SPEED) //prevent max overflow
    {
       PID_error = MAX_SPEED;
    }
    else if(PID_error < -MAX_SPEED) //prevent min overflow
    {
        PID_error = -MAX_SPEED;
    }

    if(PID_error >= 0) //writes function input (PID) to motors, if PID error is positive
    {
        motor_right(BASE_SPEED);
        motor_left(BASE_SPEED-PID_error); //turns left by slowing the left wheel
    }
    else //if PID error is negative
    {
        motor_right(BASE_SPEED+PID_error); //turns right by slowing the right wheel
        motor_left(BASE_SPEED);
    }  
}
    
void move_180(void)
{
    PORTHbits.RH3 = 1;

    motor_right(300); //turns
    motor_left(300); //turns
    Delay10KTCYx(60); //turns
    motor_left(700); //turns
    motor_right(340); //turns
    Delay10KTCYx(100); //turns

    unsigned char t = 0; //initialises temporary variable

    while(t == 0) //while line is not detected, keep turning
    {
        read_line_sensors(); //updates line sensor value
        t = line_sensor[2] + line_sensor[3]; //updates temporary variable with middle two line sensors
    }
}

void move_stop(void)
{
    motor_left(BREAK_SPEED);
    motor_right(BREAK_SPEED);
}
    
void motor_right(int power)
{
    SetDCPWM4(power); //sets power to motor
}

void motor_left(int power)
{
    SetDCPWM5(power); //sets power to motor
}

//Debug status: debugged, tested
