/* NAME: Patrick Shih
 *  Email: patrickkshih@gmail.com
 *   ID: 604580648
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <poll.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <mcrypt.h>
#include <sys/stat.h>
#include <fcntl.h>


#define BUFFER 1024

int portno;

int sockfd;
int newsockfd;

int NEW_IN;
int NEW_OUT;
int NEW_ERR;

int id;

int pipe_from[2];
int pipe_to[2];

struct pollfd fds[2];

int ENCRYPT_FLAG = 0;
const int KEY_LEN = 16;
int key_fd;
char* key = NULL;
MCRYPT e_fd, d_fd;
char* IV = NULL;

void exitStatus(){
  int shell_status = 0;
  waitpid(id, &shell_status, 0);
  
  int signal = 0x007f & shell_status;
  int status = 0xff00 & shell_status;
  
  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal, status);
}

void restore(void){
    dup2(NEW_OUT, 0);
    dup2(NEW_IN, 1);
    dup2(NEW_ERR, 2);
    close(NEW_IN);
    close(NEW_OUT);
    close(NEW_ERR);

    if (ENCRYPT_FLAG){
       mcrypt_generic_deinit(e_fd);
       mcrypt_generic_deinit(d_fd);
       mcrypt_module_close(e_fd);
       mcrypt_module_close(d_fd);
    }
}

void socketConnection(void){
    socklen_t clilen;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        fprintf(stderr,"ERROR opening socket");
        exit(1);
    }
    
    
    /* Initialize socket structure */
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
 //   fprintf(stdout,"port num: %d\n", portno);
    
    /* Now bind the host address using bind() call.*/
    int is_bind;
    if ((is_bind = bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        fprintf(stderr, "is_bind: %d\n", is_bind);
        fprintf(stderr, "ERROR on binding");
        exit(1);
    }
    
   // fprintf(stdout,"Waiting for client...\n");
    /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
     */
    //  fprintf(stdout, "sockfd: %d\n", sockfd);
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    
    
    /* Accept actual connection from the client */
    
    //  fprintf(stdout, "pre-accept");
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    //  fprintf(stdout, "Sockfd accepted?: %d\n", newsockfd);
    
    if (newsockfd < 0) {
        fprintf(stderr, "ERROR on accept");
        exit(1);
    }
    
}

void sighandler(int signal){
    if(signal == SIGPIPE){
	restore();
	exitStatus();
        exit(0);
    }
    
    if(signal == SIGINT){
        int killStatus = kill(id, SIGINT);
        if(killStatus < 0){
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
    }
}

void initPipes(){
    if (pipe(pipe_from) == -1 || pipe(pipe_to) == -1){
        restore();
        fprintf(stderr, "Creating pipes failure");
        exit(1);
    }
}

void execShell(){
    close(pipe_to[1]);
    close(pipe_from[0]);
    dup2(pipe_to[0], STDIN_FILENO);
    dup2(pipe_from[1], STDOUT_FILENO);
    dup2(pipe_from[1], STDERR_FILENO);
    close(pipe_to[0]);
    close(pipe_from[1]);
    char *execvp_argv[2];
    execvp_argv[0] = "/bin/bash";
    execvp_argv[1] = NULL;
    if (execvp("/bin/bash", execvp_argv)){
        restore();
        fprintf(stderr, "Could not access exec commands");
        exit(1);
    }
    
}

void process_input(int input_fd, int output_fd){
    int err;
    char* b = (char*) malloc(BUFFER*sizeof(char));
    int byteCount = read(input_fd, b, 1);
    if (byteCount == 0){
        restore();
        int killStatus;
        if ((killStatus = kill(id, SIGTERM)) <0){
            fprintf(stderr, "kill status unavailable");
        }
	exitStatus();
        exit(0);
    }
    if (ENCRYPT_FLAG){
	mdecrypt_generic(d_fd, b, 1); 
    }
    if (b[0] == '\003')
        sighandler(SIGINT);
    else if (b[0] == '\004')
        close(pipe_to[1]);
    else if (b[0] == '\n' || b[0] == '\r'){
        char replace[2] = "\r\n";
        err = write(pipe_to[1], &replace[1],1);
        if (err < 0){
	  restore();
	  fprintf(stderr,"Failure processing input\n");
	  exit(1);
	}
     }
     else{
        err = write(pipe_to[1], b, 1);
        if (err < 0){
	  restore();
	  fprintf(stderr,"Failure processing input\n");
	  exit(1);
	}
     } 
}

void process_input_2(int input_fd, int output_fd){
    int err;
    
    char* b = (char*) malloc(BUFFER*sizeof(char));
    int byteCount = read(input_fd, b, 1);
    if (byteCount == 0){
        restore();
        int killStatus;
        if ((killStatus = kill(id, SIGTERM)) <0){
            fprintf(stderr, "kill status unavailable");
        }
	exitStatus();
        exit(0);
    }
    if (b[0] == '\003')
        sighandler(SIGINT); 
    else if (b[0] == '\004')
        close(pipe_to[1]);
    else if (b[0] == '\n' || b[0] == '\r'){
        char replace[2] = "\r\n";
	 if (ENCRYPT_FLAG){
	     mcrypt_generic(e_fd, replace, 2);
	  }
   	 err = write(output_fd, &replace, 2);
          if (err < 0){
	    restore();
	    fprintf(stderr,"Failure processing input\n");
	    exit(1);
	  }
	}
    else{
	if (ENCRYPT_FLAG){
	    mcrypt_generic(e_fd, b, 1);
	}
        err = write(output_fd, b, 1);
        if (err < 0){
	  restore();
	  fprintf(stderr,"Failure processing input\n");
	  exit(1);
	}
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
    mcrypt_generic_init(e_fd, key, KEY_LEN, IV);
    mcrypt_generic_init(d_fd, key, KEY_LEN, IV);
}

int main (int argc, char *argv[]){    
    NEW_IN = dup(STDIN_FILENO);
    NEW_OUT = dup(STDOUT_FILENO);
    NEW_ERR = dup(STDERR_FILENO);
    
    static struct option long_opts[] = {
        {"port", required_argument, 0, 'p'},
    	{"encrypt", required_argument, 0 ,'e'}
    };
    
    int PORT_FLAG = 0;
    int op;
    while ((op =  getopt_long(argc, argv, "", long_opts, NULL)) != -1){
        switch(op){
            case 'p':
                signal(SIGINT, sighandler);
                signal(SIGPIPE, sighandler);
                PORT_FLAG = 1;
                portno = atoi(optarg);
                break;
	    case 'e':
		key_fd = open(optarg, O_RDONLY);
		if (key_fd < 0){
		    fprintf(stderr, "Failure to openn key\n");
		    exit(1);
		}
		ENCRYPT_FLAG = 1;
		set_encrypt_mode();			
		break;
            default:
                fprintf(stderr, "Correct Usage: ./lab1b-server --port=# --encrypt=my.key\n");
                exit(1);
        }
    }
    
    if (PORT_FLAG){
        socketConnection();
        
        dup2(newsockfd, STDIN_FILENO);
        dup2(newsockfd, STDOUT_FILENO);
        dup2(newsockfd, STDERR_FILENO);
        close(newsockfd);
        
        initPipes();
        
        id = fork();
        
        if (id == -1){
            restore();
            fprintf(stderr, "Failure to fork\n");
            exit(1);
        }
        
        if (id == 0){ //child
            execShell(); 
	}
        else if (id > 0){ //parent
            fds[0].fd = STDIN_FILENO;
            fds[0].events = POLLIN | POLLERR | POLLHUP;
            fds[1].fd = pipe_from[0];
            fds[1].events = POLLIN | POLLERR | POLLHUP;
            
            while(1){
                int rc;
                if((rc = poll(fds, 2, -1)) <= 0) {
                    restore();
                    fprintf(stderr, "Failure to poll\n");
                    exit(rc);
                }
                else{
                    if(fds[0].revents & POLLIN){
                        process_input(STDIN_FILENO, STDOUT_FILENO);
                    }
                    if(fds[1].revents & POLLIN){            
                        close(pipe_to[0]);
                        close(pipe_from[1]);
                        process_input_2(pipe_from[0], STDOUT_FILENO);
                    }
                    if (exitCondition()){
                        restore();
			exitStatus();

                        close(STDIN_FILENO);
                        close(STDOUT_FILENO);
                        close(STDERR_FILENO);
                        close(pipe_to[0]);
                        close(pipe_to[1]);
                        close(pipe_from[0]);
                        close(pipe_from[1]);
                        close(sockfd);
			exit(0);
                    }
                }
            }
        }
    }
    exit(0);
}
