/*
 * Intel(R) Edison-based Lego(R) car
 * Demo 1 - Basic Operation
 *
 * Description: This demonstrates the basic operations of the car.
 * 		The program asks for the turn, which can be a left turn, right turn, or straight.
 *		Then, it asks for the speed.
 *		100 is the maximum speed forward.
 *		0 is "stop".
 *		-100 is the maximum speed backward.
 *
 * Author: In Hwan "Chris" Baek
 * E-mail: chris.inhwan.baek@gmail.com
 * UCLA Wireless Health Institute
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <mraa/pwm.h>

#define MAXBUFSIZ 1024
#define CENTER 0.068f

void speed_control(mraa_pwm_context, mraa_pwm_context, float);

int main(){
	float speed, turn;
	char speed_user_input[MAXBUFSIZ];
	char turn_user_input[MAXBUFSIZ];
	mraa_pwm_context speed_pwm_in1, speed_pwm_in2, turn_pwm;
	speed_pwm_in1 = mraa_pwm_init(3);
	speed_pwm_in2 = mraa_pwm_init(5);
	turn_pwm = mraa_pwm_init(6);
	
	if (speed_pwm_in1 == NULL || speed_pwm_in2 == NULL || turn_pwm == NULL) {
		fprintf(stderr, "Failed to initialized.\n");
		return 1;
	}

	mraa_pwm_period_us(speed_pwm_in1,870); //1150Hz
	mraa_pwm_enable(speed_pwm_in1, 1);
	mraa_pwm_period_us(speed_pwm_in2,870);
	mraa_pwm_enable(speed_pwm_in2, 1);
	
	mraa_pwm_period_ms(turn_pwm,20);
   	mraa_pwm_enable(turn_pwm, 1);

	mraa_pwm_write(turn_pwm, CENTER);
	mraa_pwm_write(speed_pwm_in1, 1.0f);
	mraa_pwm_write(speed_pwm_in2, 1.0f);

	while(1){
	        printf("Turn (L, C, R): ");                                      
        	scanf("%s", turn_user_input);               
                
    		if (!strcmp(turn_user_input, "L") || !strcmp(turn_user_input, "l"))     
           		turn = CENTER - 0.015f;                 
        	else if (!strcmp(turn_user_input, "C") || !strcmp(turn_user_input, "c"))
            		turn = CENTER;                 
        	else if (!strcmp(turn_user_input, "R") || !strcmp(turn_user_input, "r"))
            		turn = CENTER + 0.015f;
		else {                                 
            		printf("Wrong turn type!\n");
 			return 1;
		}
		
		printf("Speed value (-100-100): ");
		scanf("%s", speed_user_input);
		speed = atof(speed_user_input);

		if (speed > 100 || speed < -100)
			printf("Error: Choose between -100 and 100\n");
		else {
			mraa_pwm_write(turn_pwm, turn);  
			usleep(100000);
			speed_control(speed_pwm_in1, speed_pwm_in2, speed);
		}
		sleep(1);
		speed_control(speed_pwm_in1, speed_pwm_in2, 0.0f);
	}
	return 0;
}

void speed_control(mraa_pwm_context in1, mraa_pwm_context in2, float speed) {
	speed = speed/100;                                  
        if (speed >= 0) {                                   
                mraa_pwm_write(in2, 1.0f);        
                mraa_pwm_write(in1, 1.0f - speed);
        }                                                   
        else if (speed < 0) {                               
                mraa_pwm_write(in1, 1.0f);        
                mraa_pwm_write(in2, 1.0f + speed);
        }    
}

