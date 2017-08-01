#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <poll.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>


#define BUFFER 1024

//Source: https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
struct termios saved_attributes;

void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

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

int process_input(char* buff, int size){
  
  if (buff[size-1] == '\003'){ //<C-c>
    return 2;
  }
  else if (buff[size-1] == '\004'){ //<C-d>
    reset_input_mode();
    return 1;
  }
  else if (buff[size-1] == '\n' || buff[size-1] == '\r'){
    return 3;
  }
  return 0;
}

int main (int argc, char *argv[]){ 

  static struct option long_opts[] = {
    {"shell", no_argument, 0, 's'},
  };


  int SHELL_FLAG = 0;
  int op; 
  while ((op =  getopt_long(argc, argv, "", long_opts, NULL)) != -1){
    switch(op){
      case 's':
        SHELL_FLAG = 1;
	break;
      default:
	printf("Correct Usage: ./lab1a [--shell]");
	exit(1);
    }
  }  
 
  set_input_mode(); 
  
  int i, rc, err;
  char* b = (char*) malloc(BUFFER*sizeof(char));
  char ch;
  int pipe_from[2];
  int pipe_to[2]; 
  int endProgram = 1;
  int nBytes; 


  if (SHELL_FLAG){  
//http://beej.us/guide/bgnet/output/html/multipage/pollman.html
  if (pipe(pipe_from) == -1 || pipe(pipe_to) == -1){
	fprintf(stderr, "Creating pipes failure");
	exit(1);
  }

  int id = fork();
   
  if (id == -1){
	fprintf(stderr, "Failure to fork");
	exit(1);
  }

  if (id == 0){ //child
    close(pipe_to[1]);
    close(pipe_from[0]); 
    dup2(pipe_to[0], STDIN_FILENO);
    dup2(pipe_from[1], STDOUT_FILENO);
    close(pipe_to[0]);    
    close(pipe_from[1]);
    char *execvp_argv[2];
    execvp_argv[0] = "/bin/bash";
    execvp_argv[1] = NULL;
    if (execvp("/bin/bash", execvp_argv)){
	fprintf(stderr, "Could not access exec commands");
	exit(1);
    }
  }
  else if (id > 0){ //parent
    int check;
    close(pipe_to[0]);
    close(pipe_from[1]);      
    while(1){
      struct pollfd fds[2];
      fds[0].fd = STDIN_FILENO;
      fds[0].events = POLLIN | POLLERR;
      fds[1].fd = pipe_from[0];
      fds[1].events = POLLIN | POLLERR;
      
      if((rc = poll(fds, 2, -1)) <= 0) {
	exit(rc);
      }
      else{
	if(fds[0].revents & POLLIN){
	  nBytes = read(STDIN_FILENO, b, 1);
	  if (nBytes == -1){
	    fprintf(stderr, "Failure to read from std input");
	    exit(0);
	  }	
          check = process_input(b, nBytes);
	  switch(check){
	    case 1:
	      kill(id, SIGHUP);
	      close(pipe_to[1]);
	      close(pipe_from[0]);
	      exit(0);
	      break;
	    case 2:
	      exit(0);
	      break;
	    default:
	      err = write(STDOUT_FILENO, b, 1);
 	      if (err == -1){
		fprintf(stderr, "Failure to write to std output");
		exit(0);
	      }
	      err = write(pipe_to[1], b, 1);
	      if (err == -1){
		fprintf(stderr, "Failure to write to shell");
		exit(0);
	      }

	  }
	}  
	else if(fds[1].revents & POLLIN){  
	  err = read(pipe_from[0], b, 1);
	  if (err == -1){
	    fprintf(stderr, "Failure to read from shell");
	    exit(1);
	  }
	  err = write(STDOUT_FILENO, b, 1);
	  if (err == -1){
	    fprintf(stderr, "Failure to write to std output");
	    exit(1);
	  }
	  }
        }
      }
    }
   
    int status;
    waitpid(id,&status, 0);

    if(WIFSIGNALED(status)){
	fprintf(stderr, "SHELL EXIT STATUS=%d", status & 0x007f);
	fprintf(stderr, "STATUS=%d", status & 0xff00); 
    }
  }
  else{ //no --shell argument
    int index = 0;
    char ch;
    while(1){
      err = read(STDIN_FILENO, &ch, 1);

      if (err == -1){
	fprintf(stderr, "Failure to read from std input");
	exit(1);
      }

      if (ch == '\004'){
	int j;
	for (j = 0; b[j] != '\0'; j++){
	  printf("%c",b[j]);
        }
        printf("\n");
	exit(0);
      }
      else{
	if (ch == '\015' || ch == '\012'){
	  b[index++] = '\015';
	  b[index++] = '\012';
        }
	else{
	  b[index++] = ch;
	}
      }  
    }
  }

  exit(0);
}
