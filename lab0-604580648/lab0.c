#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void sig_handler(){
  fprintf(stderr, "Forced Segmentation Fault: %s\n", strerror(errno));
  exit(4);
}

int main(int argc, char* argv[]){

  struct option long_opts[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"segfault", no_argument, NULL, 's'},
    {"catch", no_argument, NULL, 'c'},
    {0,0,0,0}
  };

  int ch, SEGFAULT_FLAG, CATCH_FLAG, INPUT_FLAG, OUTPUT_FLAG = 0;
  int ifd = 0;
  int ofd = 1;
  char* buff = (char*) malloc(sizeof(char));

  while((ch = getopt_long(argc, argv, "a:b", long_opts, NULL)) != -1){
    switch(ch){
      case 'i':
	INPUT_FLAG = 1;
	if ((ifd = open(optarg, O_RDONLY)) == -1){
	  fprintf(stderr,"Failure to open file: %s\n",strerror(errno));
	  exit(2);
	}
	break;
      case 'o':
	OUTPUT_FLAG = 1;
	  if ((ofd = creat(optarg, 0644)) == -1){
	    fprintf(stderr,"Failure to create file: %s\n",strerror(errno));
	    exit(3);
	  }
	break;
      case 's':
	SEGFAULT_FLAG = 1;
	break;
      case 'c':
	CATCH_FLAG = 1;
	break;
      default:
	printf("Correct Usage: ./lab0 [--input=filename] [--output=filename] [--segfault] [--catch]\n");
	exit(1);	
     }
  }
  if (CATCH_FLAG == 1){
    signal(SIGSEGV, sig_handler);
  }
  
  if (SEGFAULT_FLAG == 1){
    char* p = NULL;
    char temp = *p;
  }

  if (INPUT_FLAG == 1){
    close(0);
    dup(ifd);
    close(ifd);
  }

  if (OUTPUT_FLAG == 1){
    close(1);
    dup(ofd);
    close(ofd);
  }

  int rd;
  while((rd = read(0,buff,1)) > 0){ 
    write(1, buff, 1);
  } 

  exit(0);

}
