#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

int monitor_pid=0;
int monitor_status=0;//1 if running and 0 if not running
int monitor_operation_finished=0;//1 if it finished the operation and 0 otherwise
int accepting_commands=1;//1 if accepting and 0 if rejecting

void print_commands(){
  /*
    prints command list
  */
  printf("Commands:\n");
  printf("\t1 - start monitor\n");
  printf("\t2 - list hunts\n");
  printf("\t3 - list treasures\n");
  printf("\t4 - view treasure\n");
  printf("\t5 - calculate scores\n");
  printf("\t6 - stop monitor\n");
  printf("\t7 - exit\n");
}

void monitor_stopped(int sig){
  /*
    handles SIGCHLD signal
  */
  int ret_val,status;
  while ((ret_val=waitpid(-1, &status, WNOHANG))>0){//acknowledging terminated children
    if(ret_val==monitor_pid && WIFEXITED(status)){
      monitor_status=0;
      accepting_commands=1;
      printf("Monitor stopped with exit status %d\n",WEXITSTATUS(status));
    }
  }
  if(ret_val==-1 && errno!=ECHILD){
    perror(NULL);
    exit(-1);
  }
}

void monitor_started(int sig){
  /*
    handles SIGUSR1 signal (child sends SIGUSR1 when it started successfully)
  */
  monitor_status=1;
  monitor_operation_finished=0;
  printf("Monitor started\n");
}

void monitor_finished_operation(){
  /*
    sets monitor_operation_finished to 1
  */
  monitor_operation_finished=1;
}

void remove_temporary_files(){
  if(remove("./.monitor_command")==-1 && errno!=ENOENT){
    perror(NULL);
    exit(-1);
  }
  exit(0);
}

void setup_signals(){
  /*
    sets up signal handlers
  */
  struct sigaction sa;
  //sets the set of blocked signals to 0 (no signals will be blocked)
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = monitor_started;
  sa.sa_flags = SA_RESTART;//restarts any interrupted system calls
  if(sigaction(SIGUSR1, &sa, NULL) == -1){//child will send SIGUSR1 when it started succesfully
    perror(NULL);
    exit(-1);
  }

  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = monitor_finished_operation;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGUSR2, &sa, NULL) == -1){//child will send SIGUSR2 when it completed/acknowledged its task
    perror(NULL);
    exit(-1);
  }
  
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = monitor_stopped;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){//receives SIGCHLD when the a child changes states
    perror(NULL);
    exit(-1);
  }

  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = remove_temporary_files;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGINT, &sa, NULL) == -1){//received when user sends 'CTRL+C'
    perror(NULL);
    exit(-1);
  }
}

void clear_buffer(){
  /*
    clears buffer
  */
  char buffer[100];
  while(fgets(buffer,100,stdin)!=NULL){
    if(buffer[strlen(buffer)-1]=='\n') return;
  }
}

void start_monitor(int *command_file_desc,int manager_pipes[2]){
  /*
    starts a new process for the treasure_monitor
  */
  
  //initializes the pipes used for communication with treasure_hunt processes 
  if(pipe(manager_pipes)==-1){
    perror(NULL);
    exit(-1);
  }
  
  if((monitor_pid=fork())<0){
    perror(NULL);
    exit(-1);
  }
  if(monitor_pid==0){
    //child process
    if(close(manager_pipes[0])==-1){//closes read end of the pipe
      perror(NULL);
      exit(-1);
    }
    if(dup2(manager_pipes[1],1)==-1){//links stdout to the write end of the pipe
      perror(NULL);
      exit(-1);
    }
    if(close(manager_pipes[1])==-1){//closes ORIGINAL DESCRIPTOR of the write end of the pipe
      perror(NULL);
      exit(-1);
    }
    int temp_file_desc;
    if((temp_file_desc=open("./.monitor_command",O_CREAT|O_RDONLY,0644))==-1){
      perror(NULL);
      exit(-1);
    }
    if(dup2(temp_file_desc,0)==-1){//links stdin to the monitor_command file descriptor
      perror(NULL);
      exit(-1);
    }
    if(execl("./monitor","monitor",NULL)==-1){
      perror(NULL);
      exit(-1);
    }
  }
  //parent process
  int flags;
  if((flags=fcntl(manager_pipes[0],F_GETFL,0))==-1){//gets the flags of the read end of the pipe
    perror(NULL);
    exit(-1);
  }
  if(fcntl(manager_pipes[0],F_SETFL,flags|O_NONBLOCK)==-1){//sets reads from read end of the pipe to be non-blocking
    perror(NULL);
    exit(-1);
  }
  if(((*command_file_desc)=open("./.monitor_command",O_CREAT|O_WRONLY|O_TRUNC,0644))==-1){
    perror(NULL);
    exit(-1);
  }
}

int main(){
  int command_file_desc,manager_pipes[2];
  setup_signals();
  while(1){
    print_commands();

    //reads option
    char buffer[3],*endptr=NULL;
    if(fgets(buffer,3,stdin)==NULL){
      printf("Error while reading option\n");
      exit(-1);
    }
    if(!accepting_commands){
      printf("Hub is currently not accepting commands\n");
      if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
      continue;
    }
    int command=(int)strtol(buffer,&endptr,10);
    if(*endptr!='\n' || endptr==buffer){
      printf("Invalid input\n");
      if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
      continue;
    }
    
    switch(command){
      
      case 1:{
        if(!monitor_status){
	  //start monitor
	  start_monitor(&command_file_desc,manager_pipes);
	  pause();
	}
	else printf("Monitor alreeady running (PID: %d)\n",monitor_pid);
        break;
      }
	
      case 2:{
	//list hunts
	if(monitor_status){
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)2);
	  //reads from pipe
	  char pipe_buffer[PIPE_BUF+1];
	  while(1){
	    int ret=read(manager_pipes[0],pipe_buffer,PIPE_BUF);
	    if(ret==-1){
	      if(errno!=EAGAIN && errno!=EINTR){//EAGAIN - no data available, EINTR - function inerrupted
		perror(NULL);
		exit(-1);
	      }
	      if(errno==EAGAIN && monitor_operation_finished==1){
		monitor_operation_finished=0;
		break;
	      }
	    }
	    else{
	      pipe_buffer[ret]='\0';
	      printf("%s",pipe_buffer);
	    }
	  }
	}
	else printf("Monitor not running\n");
        break;
      }
	
      case 3:{
	//list treasures
        if(monitor_status){
	  //truncates the command file to prepare it for the next command
	  ftruncate(command_file_desc,0);

	  //reads hunt name
	  char hunt_name[100],hunt_name_length;
	  printf("Hunt name: ");
	  if(fgets(hunt_name,100,stdin)==NULL){
	    printf("Error while reading input\n");
	    exit(-1);
	  }
	  printf("\n");
	  if(hunt_name[strlen(hunt_name)-1]!='\n') clear_buffer();
	  hunt_name[strlen(hunt_name)-1]='\0';
	  hunt_name_length=strlen(hunt_name)+1;

	  //writes the length of the name and the name to the command file
	  if(write(command_file_desc,&hunt_name_length,1)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  if(write(command_file_desc,hunt_name,hunt_name_length)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)3);

	  //reads from pipe
	  char pipe_buffer[PIPE_BUF+1];
	  while(1){
	    int ret=read(manager_pipes[0],pipe_buffer,PIPE_BUF);
	    if(ret==-1){
	      if(errno!=EAGAIN && errno!=EINTR){//EAGAIN - no data available, EINTR - function inerrupted
		perror(NULL);
		exit(-1);
	      }
	      if(errno==EAGAIN && monitor_operation_finished==1){
		monitor_operation_finished=0;
		break;
	      }
	    }
	    else{
	      pipe_buffer[ret]='\0';
	      printf("%s",pipe_buffer);
	    }
	  }
	}
	else printf("Monitor not running\n");
        break;
      }
	
      case 4:{
	// view treasure
        if(monitor_status){
	  //truncates the command file to prepare it for the next command
	  ftruncate(command_file_desc,0);

	  //reads hunt name and parameters
	  printf("NOTE: You can provide both the Treasure ID and the User Name in that order, either User Name or Treasure ID, or none. If only one parameter is provided, if it is an integer, it is going to be considered the Treasure ID, otherwise it is going to be considered the User Name. If no parameter is provided, the 'list treasures' option will execute, using the Hunt ID provided.\n\n");
	  //reads hunt name
	  char hunt_name[100],hunt_name_length;
	  printf("Hunt name: ");
	  if(fgets(hunt_name,100,stdin)==NULL){
	    printf("Error while reading input\n");
	    exit(-1);
	  }
	  if(hunt_name[strlen(hunt_name)-1]!='\n') clear_buffer();
	  hunt_name[strlen(hunt_name)-1]='\0';

	  char input1[100],input1_length;
	  printf("Parameter 1: ");
	  if(fgets(input1,100,stdin)==NULL){
	    printf("Error while reading input\n");
	    exit(-1);
	  }
	  if(input1[strlen(input1)-1]!='\n') clear_buffer();
	  input1[strlen(input1)-1]='\0';
	  input1_length=strlen(input1)+1;
      
	  char input2[100],input2_length;
	  printf("Parameter 2: ");
	  if(fgets(input2,100,stdin)==NULL){
	    printf("Error while reading input\n");
	    exit(-1);
	  }
	  if(input2[strlen(input2)-1]!='\n') clear_buffer();
	  input2[strlen(input2)-1]='\0';
	  input2_length=strlen(input2)+1;
	  printf("\n");
	  
	  /*
	    Use case:
	    0 - got nothing
	    1 - got id or user name
	    2 - got both id and user name
	  */
	  char use_case=(input1_length!=1)+(input2_length!=1);
	  if(write(command_file_desc,&use_case,1)==-1){
	    perror(NULL);
	    exit(-1);
	  }

	  //writes hunt name length and hunt name to command file
	  hunt_name_length=strlen(hunt_name)+1;
	  if(write(command_file_desc,&hunt_name_length,1)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  if(write(command_file_desc,hunt_name,hunt_name_length)==-1){
	    perror(NULL);
	    exit(-1);
	  }

	  //writes parameter 1 length and parameter 1 to command file
	  if(input1_length!=1){
	    if(write(command_file_desc,&input1_length,1)==-1){
	      perror(NULL);
	      exit(-1);
	    }
	    if(write(command_file_desc,input1,input1_length)==-1){
	      perror(NULL);
	      exit(-1);
	    }
	  }

	  //writes parameter 2 length and parameter 2 to command file
	  if(input2_length!=1){
	    if(write(command_file_desc,&input2_length,1)==-1){
	      perror(NULL);
	      exit(-1);
	    }
	    if(write(command_file_desc,input2,input2_length)==-1){
	      perror(NULL);
	      exit(-1);
	    }
	  }
	  
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)4);

	  //reads from pipe
	  char pipe_buffer[PIPE_BUF+1];
	  while(1){
	    int ret=read(manager_pipes[0],pipe_buffer,PIPE_BUF);
	    if(ret==-1){
	      if(errno!=EAGAIN && errno!=EINTR){//EAGAIN - no data available, EINTR - function inerrupted
		perror(NULL);
		exit(-1);
	      }
	      if(errno==EAGAIN && monitor_operation_finished==1){
		monitor_operation_finished=0;
		break;
	      }
	    }
	    else{
	      pipe_buffer[ret]='\0';
	      printf("%s",pipe_buffer);
	    }
	  }
	}
	else printf("Monitor not running\n");
        break;
      }
	
      case 5:{
	//calculate scores
	int score_pipes[2];
	if(pipe(score_pipes)==-1){
	  perror(NULL);
	  exit(-1);
	}
	int child_pid;
	if((child_pid=fork())==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(child_pid==0){
	  //child process
	  if(close(score_pipes[0])==-1){//closes read end of the pipe
	    perror(NULL);
	    exit(-1);
	  }
	  if(dup2(score_pipes[1],1)==-1){//links write end of the pipe to stdout
	    perror(NULL);
	    exit(-1);
	  }
	  if(close(score_pipes[1])==-1){//closes ORIGINAL DESCRIPTOR of the write end of the pipe
	    perror(NULL);
	    exit(-1);
	  }
	  if(execl("./calculate_score_hub","./calculate_score_hub",NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	//parent process
	if(close(score_pipes[1])==-1){//closes write end of the pipe
	  perror(NULL);
	  exit(-1);
	}

	//reads from pipe
	char pipe_buffer[PIPE_BUF+1];
	int ret;
	while((ret=read(score_pipes[0],pipe_buffer,PIPE_BUF))>0){
	  pipe_buffer[ret]='\0';
	  printf("%s",pipe_buffer);
	}
	if(ret==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(close(score_pipes[0])==-1){
	  perror(NULL);
	  exit(-1);
	}
	break;
      }
	
      case 6:{
	//stop monitor
        if(monitor_status){
	  if(kill(monitor_pid,SIGUSR2)!=0){
	    perror(NULL);
	    exit(-1);
	  }
	  pause();
	  if(close(command_file_desc)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  if(remove("./.monitor_command")==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  if(close(manager_pipes[0])==-1 || close(manager_pipes[1])==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  printf("Stopping monitor\n");
	  accepting_commands=0;
	}
	else{
	  printf("Monitor not running\n");
	}
        break;
      }
	
      case 7:{
	//exit
	if(monitor_status) printf("Monitor still running (PID: %d)\n",monitor_pid);
	else return 0;
	break;
      }
	
      default:{
        printf("Invalid command\n");
        break;
      }
	
    }
  }
  return 0;
}
