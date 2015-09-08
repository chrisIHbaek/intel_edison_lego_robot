/*
 * Intel(R) Edison-based Lego(R) car
 * Demo 2 - Cloud-based control 
 *
 * Description: This program reads the JSON formatted actuation requests.
 *              The actuation requests are received by the IoT agent.
 *              This program reads the requests from the IoT agent via UDP protocol.
 *              The user can control the car from the Intel Cloud analytics dashboard.
 *
 * Authors: In Hwan "Chris" Baek and Leonardo Hernandez
 * Email: chris.inhwan.baek@gmail.com
 * UCLA Wireless Health Institute
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <mraa/gpio.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/pwm.h>

#define MAX_SIZE 1024
#define CENTER 0.068f

const char* localhost = "127.0.0.1";

void speed_control(mraa_pwm_context, mraa_pwm_context, float);

int main()
{
	int sockfd;
	float brightness;
	struct sockaddr_in server_addr;
	char response[MAX_SIZE];
	char cmd[MAX_SIZE];
	char cmd2[MAX_SIZE];                                
        FILE *fp, *fp2;                                          
        char component_string[MAX_SIZE];
	char value_string[MAX_SIZE]; 
	int value_int;

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

	mraa_pwm_write(speed_pwm_in1, 1.0f);
	mraa_pwm_write(speed_pwm_in2, 1.0f);
	mraa_pwm_write(turn_pwm, CENTER);

	//Set the protocol to mqtt and
	//restart iotkit-agent to ensure that it is running.
	system("iotkit-admin protocol mqtt");
	system("systemctl restart iotkit-agent");

	//create a socket with the following attributes:
	socklen_t len = sizeof(server_addr);	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&server_addr,sizeof(server_addr));

	//Name of the socket as agreed with the server
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(localhost);
	server_addr.sin_port = htons(41235);	
	
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        	fprintf(stderr, "Failed to bind");
        	return 1;
    	}

	while(1)	
    	{
		//Use recvfrom function to send JSON message to iotkit-agent
		if(recvfrom(sockfd, response, MAX_SIZE, 0, (struct sockaddr *)&server_addr, &len) < 0)
    		{
			fprintf(stderr, "Failed to receive actuation\n");
    			return 1;
		}
		//printf("Received JSON message: %s\n", response);


		//---This part extracts the component name----------------
		snprintf(cmd, sizeof(cmd), "echo '%s' | sed 's/\"/ /g' | tr ' ' '\n' | awk '/component/{getline; getline; print}'", response);
		fp = popen(cmd, "r");

	  	if (fp == NULL) {
    			fprintf(stderr, "Failed to run command\n" );
    			exit(1);
  		}
	
	  	while (fgets(component_string, sizeof(component_string)-1, fp) != NULL) {
  		}
  		pclose(fp);

		//---This part extracts the value-------------------------		
		snprintf(cmd2, sizeof(cmd2), "echo '%s' | sed 's/\"/ /g' | tr ' ' '\n' | awk '/value/{getline; getline; print}'", response);

                fp2 = popen(cmd2, "r");

                if (fp2 == NULL) {
                        fprintf(stderr, "Failed to run command\n" );
                        exit(1);
                }

                while (fgets(value_string, sizeof(value_string)-1, fp) != NULL) {
                }
                pclose(fp2);

		//printf(value_string);
		//---This part is for actuation
		if (!strcmp(component_string, "vehicleControl\n")) {
			if (!strcmp(value_string, "forward\n")) {
				printf("Going forward for 1 second.\n\n");
				mraa_pwm_write(turn_pwm, CENTER); //straight                                                          
                                usleep(100000);                                                                                       
                                speed_control(speed_pwm_in1, speed_pwm_in2, 100);                                                     
                                sleep(1);                                                                                             
                                speed_control(speed_pwm_in1, speed_pwm_in2, 0);                                                       
			}
			else if (!strcmp(value_string, "reverse\n")) {
				printf("Going backward for 1 second.\n\n");
				mraa_pwm_write(turn_pwm, CENTER); //reverse                                                             
                                usleep(100000);                                                                                       
                                speed_control(speed_pwm_in1, speed_pwm_in2, -100);                                                     
                                sleep(1);                                                                                             
                                speed_control(speed_pwm_in1, speed_pwm_in2, 0); 
			}
			else if (!strcmp(value_string, "left\n")) {                                                                         
				printf("Making a left turn.\n\n");
				mraa_pwm_write(turn_pwm, CENTER - 0.015f); //left                                                              
                                usleep(100000);                                                                                       
                                speed_control(speed_pwm_in1, speed_pwm_in2, 100);                                                     
                                sleep(1);                                                                                             
                                speed_control(speed_pwm_in1, speed_pwm_in2, 0);                                                                                                                                               
                        }
			else if (!strcmp(value_string, "right\n")) {                                                                         
				printf("Making a right turn.\n\n");
				mraa_pwm_write(turn_pwm, CENTER + 0.015f); //right                                                           
                                usleep(100000);                                                                                       
                                speed_control(speed_pwm_in1, speed_pwm_in2, 100);                                                    
                                sleep(1);                                                                                             
                                speed_control(speed_pwm_in1, speed_pwm_in2, 0);                                                                                                                                               
                        }
			else
				fprintf(stderr, "Wrong actuation command received!\n");
		}		
	}
	close(sockfd);
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
