/* NAME: Patrick Shih
 * Email: patrickkshih@gmail.com
 * ID: 604580648
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <poll.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <mcrypt.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER 1024


int sockfd;
int port_num;
int log_fd;
char* log_file = NULL;
struct pollfd fds[2];

int ENCRYPT_FLAG = 0;
const int KEY_LEN = 16; //twofish uses 128 bit key
int key_fd;
char* key = NULL;
MCRYPT e_fd, d_fd;
char* IV = NULL;

struct termios saved_attributes;
void reset_input_mode (void){
    tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void socketConnection(void){
    //https://www.tutorialspoint.com/unix_sockets/socket_client_example.htm
    //setting up process as TCP client
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	reset_input_mode();
        fprintf(stderr, "Opening socket error\n");
        exit(1);
    }
    if ((server = gethostbyname("localhost")) == NULL){
	reset_input_mode();       
	fprintf(stderr, "No such host \n");
        exit(1);
    }
     
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char*) &serv_addr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port_num);
    
    //  fprintf(stderr, "sockfd: %d\n",sockfd);
    
   // fprintf(stdout, "Waiting to connect to server...\n");
    int is_connect = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //  fprintf(stderr, "is_connect: %d\n", is_connect);
    if (is_connect < 0) {
	reset_input_mode();
        fprintf(stderr, "Error connecting\n");
        exit(1);
    }
    // Done creating and connecting socket
    //fprintf(stdout,"Connection Success!\n");
}

//Source: https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
void set_input_mode(void){
    struct termios tattr;
    char *name;
    
    /* Make sure stdin is a terminal. */
    if (!isatty (STDIN_FILENO))
    {
        fprintf (stderr, "Not a terminal.\n");
        exit (EXIT_FAILURE);
    }
    
    /* Save the terminal attributes so we can restore them later. */
    tcgetattr (STDIN_FILENO, &saved_attributes);
    atexit (reset_input_mode);
    
    /* Set the funny terminal modes. */
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

void process_input(int input_fd, int output_fd){
    int err;
    char* b = (char*) malloc(BUFFER*sizeof(char));
    int byteCount = read(input_fd, b, 1);
    if (byteCount == 0){
	reset_input_mode();
        exit(0);
    }
    err = write(STDOUT_FILENO, b, 1);
    if (err < 0){
	reset_input_mode();
        fprintf(stderr, "Error processing input\n");
        exit(1);
    }   

    if (ENCRYPT_FLAG){
       err = mcrypt_generic(e_fd, b, 1);
      if (err != 0){
	 reset_input_mode();
         fprintf(stderr,"encryption failure");
	 exit(1);
       }
    }
    if (log_file){
	char message_send[14] = "SENT 1 bytes ";
 	err = write(log_fd, message_send, 14);
    	if (err < 0){
	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
    	}  
	err = write(log_fd, b, 1);
	if (err < 0){
	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
        }  
	err = write(log_fd, "\n", 1);
        if (err < 0){
  	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
        }  
    }
    if (b[0] == '\r'){
        if (ENCRYPT_FLAG){
	err = mcrypt_generic(e_fd, "\n",1);
          if (err != 0){
 	    reset_input_mode();
            fprintf(stderr,"encryption failure");
	    exit(1);
          }
	}
	err = write(output_fd, "\n",1);
        if (err < 0){
  	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
        }  
    }
    write(output_fd, b, 1);
    if (err < 0){
	reset_input_mode();
        fprintf(stderr, "Error processing input\n");
        exit(1);
    }
}

void process_input_2(int input_fd, int output_fd){
    int err; 
    char* b = (char*) malloc(BUFFER*sizeof(char));
    int byteCount = read(input_fd, b, 1);
    if (byteCount == 0){
         reset_input_mode();
	 exit(0);
    } 
    if (log_file){
	char message_rec[18] = "Received 1 bytes: ";
	write(log_fd, message_rec, 18);
	if (err < 0){
	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
    	}  
	write(log_fd, b, 1);
 	if (err < 0){
	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
    	}  
	write(log_fd, "\n", 1);
        if (err < 0){
  	  reset_input_mode();
          fprintf(stderr, "Error processing input\n");
          exit(1);
        }  
    }
    if (ENCRYPT_FLAG){
	err=mdecrypt_generic(d_fd, b, 1);
	if (err != 0){
	  reset_input_mode();
          fprintf(stderr,"encryption failure");
	  exit(1);
       }
    }
    write(output_fd, b,1);
    if (err < 0){
	reset_input_mode();
        fprintf(stderr, "Error processing input\n");
        exit(1);
    }  
}

int exitCondition(){
    return fds[0].revents & POLLERR || fds[0].revents & POLLHUP || fds[1].revents & POLLERR || fds[1].revents & POLLHUP;
}

void set_encrypt_mode(){
    e_fd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    d_fd = mcrypt_module_open("twofish", NULL, "cfb", NULL);

    key = calloc(1, KEY_LEN);
    read(key_fd, key, KEY_LEN);
    close(key_fd);

    IV = malloc(mcrypt_enc_get_iv_size(e_fd));
    IV = "zzzzzzzzzzzzzzzz";
    mcrypt_generic_init(d_fd, key, KEY_LEN, IV);
    mcrypt_generic_init(e_fd, key, KEY_LEN, IV);
}

int main (int argc, char *argv[]){
    static struct option long_opts[] = {
        {"port", required_argument, 0, 'p'},
       	{"log", required_argument, 0, 'l'},
	{"encrypt", required_argument, 0, 'e'}
    };
    
    int PORT_FLAG = 0;
    int op;
    while ((op =  getopt_long(argc, argv, "", long_opts, NULL)) != -1){
        switch(op){
            case 'p':
                PORT_FLAG = 1;
                port_num = atoi(optarg);
                break;
	    case 'l':
		log_file = optarg;
        	break;
	    case 'e':
		key_fd = open(optarg, O_RDONLY);
		if (key_fd < 0){
		     fprintf(stderr, "Failure to open key");
		     exit(1);
		}
		ENCRYPT_FLAG = 1;
		break;
	    default:
                fprintf(stderr, "Correct Usage: ./lab1b-client --port=#### --log=*.txt --encrypt=my.key");
                exit(1);
        }
    }
 
    if(log_file){
	if((log_fd = dup(open(log_file, O_CREAT|O_TRUNC|O_WRONLY, 0666))) < 0){
	  fprintf(stderr, "Fail to open file\n");
          exit(1);
	}   
   }

    set_input_mode();
    
    if(ENCRYPT_FLAG){
	set_encrypt_mode();
    }
    
    int i, rc, err;
    char* b = (char*) malloc(BUFFER*sizeof(char));
    char ch;
    int endProgram = 1;
    int nBytes;
    
    if (PORT_FLAG){
        //http://beej.us/guide/bgnet/output/html/multipage/pollman.html
        //  fprintf(stderr, "PORT_FLAG TRIGGERED\n");
        socketConnection();
     //   fprintf(stdout, "Connection Success!\n");
        int check;
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN | POLLERR;
        fds[1].fd = sockfd;
        fds[1].events = POLLIN | POLLERR;
        
        while(1){
            if((rc = poll(fds, 2, -1)) <= 0) {
		reset_input_mode();
                fprintf(stderr, "Failure to poll\n");
                exit(rc);
            }
            else{
                if(fds[0].revents & POLLIN){
                    process_input(STDIN_FILENO, sockfd);
                }  
                if(fds[1].revents & POLLIN){  
                    process_input_2(sockfd, STDOUT_FILENO);
                }
		if (exitCondition()){
		    reset_input_mode();
		    exit(0);
		}
            }
        }
    }
    exit(0);
}
