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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <openssl/ssl.h>

/* logging */
FILE *log_fp;
int REPORT_FLAG = 1; // 1: continue logging, 0: stop logging, -1: quit processes

/* input arguments */
int SCALE_FLAG = 0;
int LOG_FLAG = 0;
int period = 1;

/*hardware init*/
mraa_aio_context deg;
uint16_t deg_val = 0.0;
float temp;
time_t rawtime;
struct tm* timeinfo;

int client_fd, portno;
struct sockaddr_in serv_addr;
struct hostent *server;

int server_flag = 0;
char* host;
char* id;
int PORT_FLAG = 0;
char command[64];

SSL* ssl_descriptor;
SSL_CTX* ssl_context;

void init_hardware(){
	deg = mraa_aio_init(0);
	if (deg == NULL){
		fprintf(stderr, "Failure initializing input device\n");
		exit(1);
	}
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


void* process_output(){
	time_t prev = time(0);
    char time_str[16];
	while(1){
		rawtime = time(0);
        timeinfo = localtime(&rawtime);
        strftime(time_str, sizeof(time_str), "%H:%M:%S",timeinfo);
		deg_val = mraa_aio_read(deg);
		if (rawtime >= prev + period){
			temp = adjust_temperature(deg_val);
			if (REPORT_FLAG){ 
				if (LOG_FLAG){
					if (temp < 10){
						fprintf(log_fp, "%s 0%.1f\n", time_str, temp);
                        fflush(log_fp);
                    }
					else{
						fprintf(log_fp, "%s %.1f\n", time_str, temp);
                        fflush(log_fp);
                    }
				}
				if (server_flag){
					char text[16];
					int buffer_len;
					int SSL_write_rc;
					if (temp < 10){	
						buffer_len = sprintf(text, "%s 0%.1f\n", time_str, temp);
						SSL_write_rc = SSL_write(ssl_descriptor, text, buffer_len);
					}
					else{
						buffer_len = sprintf(text, "%s %.1f\n", time_str, temp);
						SSL_write_rc = SSL_write(ssl_descriptor, text, buffer_len);
					}
				}
			}
			prev = rawtime;
		}
	}
	return NULL;
}

int main(int argc, char* argv[]){

	static struct option long_opts[]={
		{"id", required_argument, 0, 'i'},
		{"host", required_argument, 0, 'h'},
		{"log", required_argument, 0, 'l'},
		{0,0,0,0}
	};

	int op,rc;
	int param_index = 1;
	while (param_index < argc){
		if ((op = getopt_long(argc, argv, "", long_opts, NULL)) != -1){
			switch(op){
            	case 'i': //id
            	if (strlen(optarg) != 9){
            		fprintf(stderr, "ID must be 9 digits long");
            		exit(1);
            	}
            	id = optarg;
            	break;
            	case 'h': //host
            	host = optarg;
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
            	fprintf(stderr, "Correct Usage: ./lab4c_tcp --id=# --host=* --log=*\n");
            	exit(1);
            }
        }
        else{
        	if(!PORT_FLAG){
        		portno = atoi(argv[param_index]);
        		PORT_FLAG = 1;
        	}
        }
        param_index++;
    }

    if(host != NULL && id != NULL)
    {
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	ssl_context = SSL_CTX_new(TLSv1_client_method());
	ssl_descriptor = SSL_new(ssl_context);
    	server = gethostbyname(host);
    	client_fd = socket(AF_INET, SOCK_STREAM, 0);
    	serv_addr.sin_family = AF_INET;
    	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    	serv_addr.sin_port = htons(portno);
    	if (connect(client_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
    		perror("Invalid host or port number");
    		exit(1);
    	}
    	else 
    	{
    		server_flag = 1;
    	}
    	SSL_set_fd(ssl_descriptor, client_fd);
	SSL_connect(ssl_descriptor);
	if (server_flag){
		int buffer_len;
		char text[100];
		int SSL_write_rc;
		buffer_len = sprintf(text, "ID=%s\n", id);
		if (buffer_len < 0){
			fprintf(stderr, "Fail to put id into buffer\n");
		}
		SSL_write_rc = SSL_write(ssl_descriptor, text, buffer_len);
	        if (SSL_write_rc < 0 ){
			fprintf(stderr, "SSL write failure\n");
			exit(2);
		}
	}	
    }
    else
    {
    	fprintf(stderr,"Unspecified host or port number");
    	exit(1);
    }


    init_hardware();
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, process_output, NULL);

    struct pollfd fds[1];
    fds[0].fd = client_fd;
    fds[0].events = POLLIN | POLLERR;

    char command[16];
    char substr[16];
    char time_str[16];

    while(1){
    	if((rc = poll(fds, 1, -1)) <= 0){
    		fprintf(stderr, "Polling error\n");
    		exit(1);
    	}
    	else {
    		if (fds[0].revents & POLLIN) {
			char c[512];
			ssize_t num_bytes;
			num_bytes = SSL_read(ssl_descriptor, command, 255);
    			if(strncmp(command,"OFF\n", 4) == 0){
                    fprintf(log_fp, "OFF\n");
                    fflush(log_fp);

                    rawtime = time(0);  
                    timeinfo = localtime(&rawtime);
                    strftime(time_str, sizeof(time_str), "%H:%M:%S",timeinfo); 

    				if (LOG_FLAG){
    					fprintf(log_fp, "%s SHUTDOWN\n", time_str);
                        fflush(log_fp);
    				}
                    if (server_flag){
                        int buffer_len;
			char text[100];
			int SSL_write_rc;
			buffer_len = sprintf(text,"%s SHUTDOWN\n", time_str);
         		if (SSL_write_rc < 0 ){
				fprintf(stderr, "SSL write failed\n");
				exit(2);
			}
	 	    }
    				exit(0);
    			}
    			else if (strncmp(command,"STOP\n",5) == 0){
    				REPORT_FLAG = 0;
    				if (LOG_FLAG){
    					fprintf(log_fp, "STOP\n");
                        fflush(log_fp);
                    }
    			}
    			else if (strncmp(command,"START\n",6) == 0){
    				REPORT_FLAG = 1;
    				if (LOG_FLAG){
    					fprintf(log_fp, "START\n");
                        fflush(log_fp);
                    }
    			}
    			else if (strncmp(command,"SCALE=F\n",8) == 0){
    				SCALE_FLAG = 0;
    				if (LOG_FLAG){
    					fprintf(log_fp, "SCALE=F\n");
                        fflush(log_fp);
                    }
    			}
    			else if (strncmp(command,"SCALE=C\n",8) == 0){
    				SCALE_FLAG = 1;
    				if (LOG_FLAG){
    					fprintf(log_fp, "SCALE=C\n");
                        fflush(log_fp);
                    }
    			}
    			else if (strncmp(command, "PERIOD=", 7) == 0) {
    				int i;
				if (sscanf(command, "PERIOD=%d\n", &i) == EOF){
    					if(LOG_FLAG){
						fprintf(log_fp, "Bad command for period\n");
						fflush(log_fp);
					}
				}
				if (LOG_FLAG){
					fprintf(log_fp, "PERIOD=%d\n", i);
					fflush(log_fp);
				}
				period = i;
			
			}
    			else {
    				if (LOG_FLAG){
    					fprintf(log_fp, "INVALID COMMAND: %s\n",command);
                        fflush(log_fp);
    				}
    				exit(1);
    			}
			
    		}
    	}
    }

    exit(0);
}


