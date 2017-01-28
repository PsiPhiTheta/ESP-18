//0. INFORMATION & NOTES

	/////////////////////////////////
	// ESP Control Algorithm		/
	// Contributors: Thomas Hollis	/
	// Version number: 1.0000 		/
	/////////////////////////////////

//1. Preprocessor directives
#include "xc_config_settings.h"
#include "delays.h"
#include "pconfig.h"
#include "adc.h" //remember to enable File/Project Properties/Linker/Link in periferal
#include "stdint.h"

//2. DEFINITIONS
	//#define PI 3.1415926
	//#define E 2.71828182846
	//#define E0 0.00000000000885
	
//3. FUNCTIONS
void bugg_startup(void){
	
}

void recovery_line_recovery(){
	while (/* line_detected == 0 */) {
		//scan left & right 
	}
	else {
		buggy_line_follow();
	}
}

void motor_turn (unsigned int turn){
	
}
void interupt_drive_ramp (){
	if(speed_of_wheel << max_speed){
        //apply power to wheel to push buggy up ramp
	}
    else{
		speed_of_wheel = max_speed;
	}
void interupt_apply_break()	{
	if(speed_of_wheel >> max_speed){
		speed_of_wheel = 0; 
	}
	else speed_of_wheel = max_speed;
}

}

//4. MAIN
void main (void) {

	buggy_startup();
	
	while (/* end_detected == 0 */){
		if (/* line_detected == 1 */) {
			buggy_line_follow();
		}
		else {
			buggy_line_recovery();
		}
	}
	
	motor_turn(180);
	
	while (/* end_detected == 0 */){
		if (/* line_detected == 1 */) {
			buggy_line_follow();
		}
		else {
			buggy_line_recovery();
		}
	}
	
	buggy_off();
		

}

//5. Debug status: debugged	
