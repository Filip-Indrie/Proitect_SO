#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "treasure_struct.h"

int score_operation_finished=0;//1 when the score child finished its operation and 0 otherwise

void score_terminated(){
  /*
    sets score_operation_finished to 1 and reaps the child
  */
  if(waitpid(-1,NULL,0)==-1){
    perror(NULL);
    exit(-1);
  }
  score_operation_finished=1;
}

void setup_signals(){
  struct sigaction sa;
  sa.sa_handler = score_terminated;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){//receives SIGCHLD when a child changes states
    perror(NULL);
    exit(-1);
  }
}

int main(){
  setup_signals();
  
  int found=0;
  DIR *current_dir;
  if((current_dir=opendir("."))==NULL){
    perror(NULL);
    exit(-1);
  }

  //parses through the current directory
  struct dirent *current_dir_entry;
  errno=0;
  while((current_dir_entry=readdir(current_dir))!=NULL){
    if(strcmp(current_dir_entry->d_name,".")!=0 && strcmp(current_dir_entry->d_name,"..")!=0){
      struct stat entry_stat;
      int stat_return_val=stat(current_dir_entry->d_name,&entry_stat);
      if(stat_return_val!=0){
	perror(NULL);
	exit(-1);
      }
      else if(S_ISDIR(entry_stat.st_mode)){//current entry is a directory
	char treasures_file_path[PATH_MAX]="./";
	strcat(treasures_file_path,current_dir_entry->d_name);
	strcat(treasures_file_path,"/treasures.bin");
	struct stat treasures_file_stat;
	int treasures_file_stat_return_val=stat(treasures_file_path,&treasures_file_stat);
	if(treasures_file_stat_return_val==0 && S_ISREG(treasures_file_stat.st_mode)){//current entry in the 'supposed hunt' directory is a regular file and is named 'treasures.bin'
	  found=1;
	  printf("%s:\n",current_dir_entry->d_name);
	  if(treasures_file_stat.st_size%sizeof(treasure_t)!=0){//the size of the file isn't a multiple of the size of treasure_t which shouldn't happen
	    printf("Treasures file corrupted.\n");
	    continue;
	  }
	  
	  int treasures_file_desc;
	  if((treasures_file_desc=open(treasures_file_path,O_RDONLY))==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  
	  int score_pipes[2];
	  if(pipe(score_pipes)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  int child_pid;
	  if((child_pid=fork())<0){
	    perror(NULL);
	    exit(-1);
	  }
	  if(child_pid==0){
	    //child process
	    if(close(score_pipes[0])==-1){//closes read end of the pipe
	      perror(NULL);
	      exit(-1);
	    }
	    if(dup2(score_pipes[1],1)==-1){//links stdout to the write end of the pipe
	      perror(NULL);
	      exit(-1);
	    }
	    if(close(score_pipes[1])==-1){//closes ORIGINAL DESCRIPTOR of the write end of the pipe
	      perror(NULL);
	      exit(-1);
	    }
	    if(dup2(treasures_file_desc,0)==-1){//links stdin to the treasures file
	      perror(NULL);
	      exit(-1);
	    }
	    if(execl("./calculate_score","./calculate_score",NULL)==-1){
	      perror(NULL);
	      exit(-1);
	    }
	  }
	  //parent process
	  if(close(treasures_file_desc)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  
	  int flags;
	  if((flags=fcntl(score_pipes[0],F_GETFL,0))==-1){//gets the flags of the 'read end' of the pipe
	    perror(NULL);
	    exit(-1);
	  }
	  if(fcntl(score_pipes[0],F_SETFL,flags|O_NONBLOCK)==-1){//sets reads from 'read end' of the pipe to be non-blocking
	    perror(NULL);
	    exit(-1);
	  }

	  //reads from pipe
	  char pipe_buffer[PIPE_BUF+1];
	  while(1){
	    int ret=read(score_pipes[0],pipe_buffer,PIPE_BUF);
	    if(ret==-1 || ret==0){
	      if(errno!=EAGAIN && errno!=EINTR){//EAGAIN - no data available, EINTR - function interrupted
		perror(NULL);
		exit(-1);
	      }
	      if(errno==EAGAIN && score_operation_finished==1){
		score_operation_finished=0;
		break;
	      }
	    }
	    else{
	      pipe_buffer[ret]='\0';
	      printf("%s",pipe_buffer);
	    }
	  }
	}
	else if(treasures_file_stat_return_val!=0 && errno!=ENOENT){
	  perror(NULL);
	  exit(-1);
	}
      }
    }
    errno=0;
  }
  if(errno || closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
  if(!found) printf("No hunts found\n");
  return 0;
}
