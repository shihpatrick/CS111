#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mraa/aio.h>
#include <mraa/i2c.h>
#include <poll.h>
#include <pthread.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

/* logging */
FILE *log_fp;
int REPORT_FLAG = 1; // 1: continue logging, 0: stop logging, -1: quit processes

/* input arguments */
int SCALE_FLAG = 0;
int LOG_FLAG = 0;
int period = 1;

/*hardware init*/
mraa_aio_context deg;
mraa_gpio_context button;
uint16_t deg_val = 0.0;
float temp;
time_t rawtime;
struct tm* timeinfo;


void init_hardware(){
    deg = mraa_aio_init(0);
    if (deg == NULL){
        fprintf(stderr, "Failure initializing input device\n");
        exit(1);
    }
    button = mraa_gpio_init(3);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
}

int adjust_temperature(int deg_val){
    float R = 0.0;
    float temp;
    R = 1023.0/deg_val-1.0;
    temp = 1.0/(log(R)/4275+1/298.15)-273.15;
    temp = (temp * 9)/5 + 32;
    if (SCALE_FLAG)
        temp = 5*(temp - 32)/9;
    return temp;
}

void print_time(time_t rawtime){
    char time_str[16];
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%H:%M:%S",timeinfo);
    if (LOG_FLAG)
    	fprintf(log_fp, "%s ", time_str);
    else
	fprintf(stdout, "%s ", time_str);
}

void* process_output(){
    time_t prev = time(0);
    
    while(1){
        rawtime = time(0);
        deg_val = mraa_aio_read(deg);
        if (rawtime >= prev + period){
            temp = adjust_temperature(deg_val);
            if (REPORT_FLAG){ 
                if (LOG_FLAG){
                    print_time(rawtime);
                    if (temp < 10)
			fprintf(log_fp, "0%.1f\n",temp);
		    else
			fprintf(log_fp, "%.1f\n", temp);
                }
                else{
                    print_time(rawtime);
                    if (temp < 10)
		       fprintf(stdout, "0%.1f\n", temp);
                    else
			fprintf(stdout, "%.1f\n",temp);
		}
	    }
            prev = rawtime;
	}
        if (mraa_gpio_read(button)){
            if (LOG_FLAG){
                print_time(rawtime);
		fprintf(log_fp, "SHUTDOWN\n");
                exit(0);
            }
            mraa_aio_close(deg);
            mraa_gpio_close(button);
            exit(0);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]){
    
    static struct option long_opts[]={
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's'},
        {"log", required_argument, 0, 'l'},
    	{0,0,0,0}
    };
    
    int op,rc;
    
    while ((op = getopt_long(argc, argv, "", long_opts, NULL)) != -1){
        switch(op){
            case 'p': //period
                period = atoi(optarg);
                if (period <= 0){
                    fprintf(stderr, "Period must be positive integer\n");
                    exit(1);
                }
                break;
            case 's': //scale
                if (*optarg == 'C')
                    SCALE_FLAG = 1;
                else if (*optarg == 'F')
                    SCALE_FLAG = 0;
                else {
                    fprintf(stderr, "Correct Usage: --scale=C|F\n");
                    exit(1);
                }
                break;
            case 'l': //logfile
                log_fp = fopen(optarg,"w");
                if (!log_fp){
                    fprintf(stderr, "Failure to make logfile\n");
                    exit(1);
                }
                LOG_FLAG = 1;
                break;
            default:
                fprintf(stderr, "Correct Usage: ./lab4b --period=# --scale=C|F --log=*\n");
                exit(1);
        }
    }
    
    init_hardware();
    
    deg_val = mraa_aio_read(deg);
    temp = adjust_temperature(deg_val);
    rawtime = time(0); 
    if (LOG_FLAG){
        print_time(rawtime);
	if (temp < 10)
	    fprintf(log_fp, "0%.1f\n",temp);
	else
	    fprintf(log_fp, "%.1f\n", temp);
    }
    else{
        print_time(rawtime);
        if (temp < 10)
	   fprintf(stdout, "0%.1f\n",temp);
	else	
	   fprintf(stdout, "%.1f\n", temp);
    }
    
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, process_output, NULL);
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN | POLLERR;
    char command[16];
    char substr[16];
    while(1){
        if((rc = poll(fds, 1, -1)) <= 0){
            fprintf(stderr, "Polling error\n");
            exit(1);
        }
        else {
            if (fds[0].revents & POLLIN) {
                scanf("%s", command);
                
                if(strcmp(command,"OFF") == 0){
                    if (LOG_FLAG){
                        fprintf(log_fp, "OFF\n");
       		        rawtime = time(0);   
			print_time(rawtime);
			fprintf(log_fp, "SHUTDOWN\n");
		    }
		    exit(0);
		}
                else if (strcmp(command,"STOP") == 0){
                    REPORT_FLAG = 0;
                    if (LOG_FLAG)
                        fprintf(log_fp, "STOP\n");
                }
                else if (strcmp(command,"START") == 0){
                    REPORT_FLAG = 1;
                    if (LOG_FLAG)
                        fprintf(log_fp, "START\n");
                }
                else if (strcmp(command,"SCALE=F") == 0){
                    SCALE_FLAG = 0;
                    if (LOG_FLAG)
                        fprintf(log_fp, "SCALE=F\n");
                }
                else if (strcmp(command,"SCALE=C") == 0){
                    SCALE_FLAG = 1;
                    if (LOG_FLAG)
                        fprintf(log_fp, "SCALE=C\n");
                }
                else if (strncmp(command, "PERIOD=", 7) == 0) {
                    int i;
                    strcpy(substr, &command[7]);
                    int len = strlen(substr);
                    for (i = 0; i < len; i++) {
                        if (!isdigit(substr[i])) {
                            if (LOG_FLAG)
                                fprintf(log_fp, "Invalid argument for PERIOD\n");
                            exit(1);
                        }
                    }
                    period = atoi(substr);
		    if (period < 0){
			fprintf(log_fp, "INVALID PERIOD #\n");
			exit(1);
		    }
                    if (LOG_FLAG)
                        fprintf(log_fp, "PERIOD=%d\n", period);
                }
                else {
                    if (LOG_FLAG){
                        fprintf(log_fp, "INVALID COMMAND: %s\n",command);
		    }
		exit(1);
                }
            }
        }
    }
    
    exit(0);
}
