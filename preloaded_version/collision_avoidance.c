/*
 * Intel(R) Edison-based Lego(R) Car
 * Demo 3: Collision avoidance
 *
 * Description: This program controls the car autonomously. 
 *		This demo requires the ultrasonic sensor module.
 *		When the sensor detects an obstacle, the car will steer automatically to avoid collision.
 *
 * Authors: Chris Baek and Jianfeng Wang
 * E-mail: Chris - chris.inhwan.baek@gmail.com
 *	   Jianfeng - pkueewjf@gmail.com
 * UCLA Wireless Health Institute
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <mraa/pwm.h>
#include <mraa/gpio.h>
#include <sys/time.h>
#include <math.h>

#define MAXBUFSIZ 1024
#define CENTER 0.068f
#define LEFT 0.053f
#define RIGHT 0.083f
#define LEFTMAX 0.048f
#define RIGHTMAX 0.088f

sig_atomic_t volatile isrunning = 1;

void speed_control(mraa_pwm_context, mraa_pwm_context, float);
void u_turn(mraa_pwm_context, mraa_pwm_context, mraa_pwm_context);
double get_distance(mraa_gpio_context, mraa_gpio_context);
void do_when_interrupted(int);
int case_detection(double, double, double, double, double);
void do_case_1(double, double, mraa_pwm_context, mraa_pwm_context, mraa_pwm_context);
void do_case_2(double, mraa_pwm_context, mraa_gpio_context, mraa_gpio_context);
void do_case_3(double, mraa_pwm_context, mraa_gpio_context, mraa_gpio_context);
void do_case_4(mraa_pwm_context, mraa_gpio_context, mraa_gpio_context);
void do_case_5(mraa_pwm_context, mraa_gpio_context, mraa_gpio_context);
void do_case_6(mraa_pwm_context, mraa_pwm_context, mraa_pwm_context);
void do_case_7(mraa_pwm_context, mraa_pwm_context, mraa_pwm_context, int);

int main(){
	float speed, turn;
	char speed_user_input[MAXBUFSIZ];
	char turn_user_input[MAXBUFSIZ];
	int i, case_num, averageTimes = 3, speed_flag = 1;
	double distance_l, distance_c, distance_r, distance_up_l, distance_up_r;
	mraa_gpio_context trig_l, echo_l, trig_c, echo_c, trig_r, echo_r, trig_up_l, echo_up_l, trig_up_r, echo_up_r;
	mraa_pwm_context speed_pwm_in1, speed_pwm_in2, turn_pwm;

	signal(SIGINT, &do_when_interrupted);
        
	trig_up_l = mraa_gpio_init(4);
	echo_up_l = mraa_gpio_init(7);
	trig_up_r = mraa_gpio_init(1);
	echo_up_r = mraa_gpio_init(2);
	trig_l = mraa_gpio_init(8);                                                     
	echo_l = mraa_gpio_init(9);
	trig_c = mraa_gpio_init(10);
	echo_c = mraa_gpio_init(11);
	trig_r = mraa_gpio_init(12);
	echo_r = mraa_gpio_init(13);
	speed_pwm_in1 = mraa_pwm_init(3);
	speed_pwm_in2 = mraa_pwm_init(5);
	turn_pwm = mraa_pwm_init(6);
	
	if (trig_up_l == NULL || echo_up_l == NULL || trig_up_r == NULL || echo_up_r == NULL || trig_c == NULL || echo_c == NULL || trig_l == NULL || echo_l == NULL || trig_r == NULL || echo_r == NULL || speed_pwm_in1 == NULL || speed_pwm_in2 == NULL || turn_pwm == NULL) {
		fprintf(stderr, "Failed to initialized.\n");
		return 1;
	}

        mraa_gpio_dir(trig_up_l, MRAA_GPIO_OUT);
	mraa_gpio_dir(echo_up_l, MRAA_GPIO_IN);                                                                        
	mraa_gpio_dir(trig_up_r, MRAA_GPIO_OUT);
	mraa_gpio_dir(echo_up_r, MRAA_GPIO_IN);
	mraa_gpio_dir(trig_l, MRAA_GPIO_OUT);                                   
	mraa_gpio_dir(echo_l, MRAA_GPIO_IN);
	mraa_gpio_dir(trig_c, MRAA_GPIO_OUT);
	mraa_gpio_dir(echo_c, MRAA_GPIO_IN);
	mraa_gpio_dir(trig_r, MRAA_GPIO_OUT);
	mraa_gpio_dir(echo_r, MRAA_GPIO_IN);

	//Lego(R) M-Motor configuration
	mraa_pwm_period_us(speed_pwm_in1,870); //1150Hz
	mraa_pwm_enable(speed_pwm_in1, 1);
	mraa_pwm_period_us(speed_pwm_in2,870);
	mraa_pwm_enable(speed_pwm_in2, 1);

	//Servo configuration
	mraa_pwm_period_ms(turn_pwm, 20);
   	mraa_pwm_enable(turn_pwm, 1);

	//At the start, the car will not move and the steering will be at center.
	speed_control(speed_pwm_in1, speed_pwm_in2, 0.0f);
	mraa_pwm_write(turn_pwm, 0.067f);

	sleep(1);
	
	speed_control(speed_pwm_in1, speed_pwm_in2, 100.0f);
		
	while (isrunning == 1){
		distance_up_l = get_distance(trig_up_l, echo_up_l);
		distance_up_r = get_distance(trig_up_r, echo_up_r);
		distance_l = get_distance(trig_l, echo_l);	
		distance_c = get_distance(trig_c, echo_c);
		distance_r = get_distance(trig_r, echo_r);
	
		//slow down when there is an obstacle near by.		
		if (distance_up_l < 70 || (distance_up_r < 70 && distance_up_r > 10)|| distance_l < 50 || distance_c < 50 || distance_r < 50){
			speed_flag = 0;
			speed_control(speed_pwm_in1, speed_pwm_in2, 70);
		}

		//printf("%lf %lf %lf %lf %lf\n", distance_up_l, distance_up_r, distance_l, distance_c, distance_r);
			
		case_num = case_detection(distance_up_l, distance_up_r, distance_l, distance_c, distance_r);
		//printf("case is %d\n", case_num);
			
		switch(case_num){
			case 1:
				do_case_1(distance_l, distance_r, turn_pwm, speed_pwm_in1, speed_pwm_in2);
				break;
			case 2:
				do_case_2(distance_l, turn_pwm, trig_l, echo_l); 
				break;
			case 3:
				do_case_3(distance_r, turn_pwm, trig_r, echo_r);
				break;
			case 4:
				do_case_4(turn_pwm, trig_l, echo_l);
				break;
			case 5:
				do_case_5(turn_pwm, trig_r, echo_r);
				break;
			case 6:
				do_case_6(speed_pwm_in1, speed_pwm_in2, turn_pwm);
				break;
			default: 
				do_case_7(speed_pwm_in1, speed_pwm_in2, turn_pwm, speed_flag);
				break;
		}
	}//end of while
        
	speed_control(speed_pwm_in1, speed_pwm_in2, 0.0f);
	return 0;
}

void do_case_1(double left, double right, mraa_pwm_context turn, mraa_pwm_context in1, mraa_pwm_context in2){
	speed_control(in1, in2, -100);
	sleep(1);
	speed_control(in1, in2, 100);
	if ((left - right) > 0.00001)
		mraa_pwm_write(turn, LEFT);
	else
		mraa_pwm_write(turn, RIGHT);
	sleep(1);
	usleep(500000);
}

void do_case_2(double left, mraa_pwm_context turn, mraa_gpio_context trig, mraa_gpio_context echo){
	int counter = 0;
	double previous = 1.0, current = 0, dutyCycle;
	while (counter < 3){
		while (current <= previous){
			counter = 0;
			previous = get_distance(trig, echo);
			dutyCycle = mraa_pwm_read(turn);
			dutyCycle += 0.005f;
			if (dutyCycle < RIGHTMAX)
				mraa_pwm_write(turn, dutyCycle);
			usleep(200000);
			current = get_distance(trig, echo);
			if (current > 80) break;
		}
		previous = current;
		usleep(150000);
		current = get_distance(trig, echo);
		if (current > 80)
			break;
		if (current > previous)
			counter += 1;
	}
	mraa_pwm_write(turn, CENTER);
}

void do_case_3(double right, mraa_pwm_context turn, mraa_gpio_context trig, mraa_gpio_context echo){
	int counter = 0;
	double previous = 0, current, dutyCycle;
	current = right;
	while (counter < 3){
		while (current <= previous){
			counter = 0;
			previous = get_distance(trig, echo);
			dutyCycle = mraa_pwm_read(turn);
			dutyCycle -= 0.005f;
			if (dutyCycle > LEFTMAX){
				mraa_pwm_write(turn, dutyCycle);
			}
			usleep(200000);
			current = get_distance(trig, echo);
			if (current > 80) break;
		}
		previous = current;
		usleep(150000);
		current = get_distance(trig, echo);
		if (current > 80)
			break;
		if (current > previous)
			counter += 1;
	}
	mraa_pwm_write(turn, CENTER);
}

void do_case_4(mraa_pwm_context turn, mraa_gpio_context trig_l, mraa_gpio_context echo_l){
	mraa_pwm_write(turn, RIGHT);
	usleep(600000);
	//sleep(1);
	double left = get_distance(trig_l, echo_l);
	do_case_2(left, turn, trig_l, echo_l);
}

void do_case_5(mraa_pwm_context turn, mraa_gpio_context trig_r, mraa_gpio_context echo_r){
	mraa_pwm_write(turn, LEFT);
	usleep(600000);
	//sleep(1);
	double right = get_distance(trig_r, echo_r);
	do_case_3(right, turn, trig_r, echo_r);
}

void do_case_6(mraa_pwm_context in1, mraa_pwm_context in2, mraa_pwm_context turn){
	mraa_pwm_write(turn, CENTER);
	speed_control(in1, in2, -100.0f);
	sleep(2);
	u_turn(in1, in2, turn);
}

void do_case_7(mraa_pwm_context in1, mraa_pwm_context in2, mraa_pwm_context turn, int speed_flag){
	if (speed_flag == 0){
		speed_flag = 1;
		mraa_pwm_write(turn, CENTER);
		speed_control(in1, in2, 100.0f);
	}
}

int case_detection(double upLeft, double upRight, double left, double Center, double right){
	int returnVal;
	if (upLeft < 40 && upRight < 40 && upRight > 10)
		return 6;
	else if (upLeft < 40 && upRight > 50)
		return 4;
	else if (upLeft > 50 && upRight < 40 && upRight > 10)
		return 5;
	if (Center <= 40 && left > 50 && right > 50)
		returnVal = 1;
	else if (Center > 50 && left <= 30 && right > 50)
		returnVal = 2;
	else if (Center > 50 && left > 50 && right <= 30)
		returnVal = 3;
	else if (Center <= 50 && left <= 50 && right <= 50)
		returnVal = 6;
	else if (Center <= 50 && left <= 50 && right > 50)
		returnVal = 4;
	else if (Center <= 50 && left > 50 && right <= 50)
		returnVal = 5;
	else
		returnVal = 7;
	return returnVal;
}

void speed_control(mraa_pwm_context in1, mraa_pwm_context in2, float speed) {
	speed = speed / 100;                                  
        if (speed >= 0) {                                   
                mraa_pwm_write(in2, 1.0f);        
                mraa_pwm_write(in1, 1.0f - speed);
        }                                                   
        else if (speed < 0) {                               
                mraa_pwm_write(in1, 1.0f);        
                mraa_pwm_write(in2, 1.0f + speed);
        }    
}

void u_turn(mraa_pwm_context in1, mraa_pwm_context in2, mraa_pwm_context turn) {
	mraa_pwm_write(turn, 0.050f);
	usleep(100000);
	speed_control(in1, in2, -70);
	sleep(1);

	mraa_pwm_write(turn, 0.084f);
	usleep(100000);
	speed_control(in1, in2, 100);
	sleep(1);

    	mraa_pwm_write(in1, 1.0f);                                                        
    	mraa_pwm_write(in2, 1.0f); 
	usleep(100000);

	mraa_pwm_write(turn, 0.050f);
	usleep(100000);
	speed_control(in1, in2, -100);
	sleep(1);

	mraa_pwm_write(in1, 1.0f);                                    
    	mraa_pwm_write(in2, 1.0f); 
	usleep(100000);	

	mraa_pwm_write(turn, 0.084f);                                                     
    	usleep(100000);                                                                   
    	speed_control(in1, in2, 100);                                                     
    	sleep(1);                                                                         
                                                                                          
    	mraa_pwm_write(in1, 1.0f);                                                        
    	mraa_pwm_write(in2, 1.0f);                                                        
    	usleep(100000);                                                                   
                                                                                          
    	mraa_pwm_write(turn, 0.050f);                                                     
    	usleep(100000);                                                                   
    	speed_control(in1, in2, -57);                                                    
    	sleep(1);                                                                         
                                                                                          
    	mraa_pwm_write(in1, 1.0f);                                                        
    	mraa_pwm_write(in2, 1.0f);                                                        
    	usleep(100000);  	

	mraa_pwm_write(turn, CENTER);
	speed_control(in1, in2, 100);
	usleep(100000);	
}

double get_distance(mraa_gpio_context trigger, mraa_gpio_context echo)
{
	struct timeval startTime, endTime;
        double time_taken;
	double distance;

        mraa_gpio_write(trigger, 0);
        usleep(5);
        mraa_gpio_write(trigger, 1);
        usleep(11);
        mraa_gpio_write(trigger, 0);
	
	while (mraa_gpio_read(echo) == 0);
	gettimeofday(&startTime, NULL);
	
	while (mraa_gpio_read(echo) == 1);
	gettimeofday(&endTime, NULL);
        
	time_taken = 1000000.0 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
        distance = (time_taken + 0.00) / 58.82;
	while (time_taken < 30000 && time_taken > 0){
		gettimeofday(&endTime, NULL);
		time_taken = 1000000.0 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_usec - startTime.tv_usec;
	}
        return distance;
}

void do_when_interrupted(int sig) {
	if (sig == SIGINT) 
		isrunning = 0;
}

