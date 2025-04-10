#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define NAME_LENGTH 50
#define CLUE_LENGTH 100

typedef struct{
  int treasure_id,value;
  float latitude,longitude;
  char user_name[NAME_LENGTH],clue[CLUE_LENGTH];
}treasure_t;

void print_usage_info(){
  printf("Usage: ./treasure_hunt <operation> <arg1> <arg2> ...\n");
  printf("Operations:\n");
  printf("\t--add <hunt_id> --> adds a new treasure to the specified hunt\n");
  printf("\t--list <hunt_id> --> lists all treasures in the specified hunt\n");
  printf("\t--view <hunt_id> <treasure_id> --> lists the details of all treasures with the specified id in the specified hunt\n");
  printf("\t--remove_treasure <hunt_id> <treasure_id> --> removes the treasures with the specified if from the specified hunt\n");
  printf("\t--remove_hunt <hunt_id> --> removes the specified hunt\n");
}

int read_treasure_from_file(int treasure_file_desc,treasure_t *treasure){
  /*
    returns 1 if all data was read and
    0 if read data is incomplete or
    an error occurred
  */
  /*
    reading and checking if all bytes were read
  */
  int bytes_read;
  if((bytes_read=read(treasure_file_desc,treasure,sizeof(treasure_t)))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(240);
  }
  if(bytes_read!=sizeof(treasure_t)) return 0;
  return 1;
}

void write_treasure_to_file(int treasure_file_desc,treasure_t treasure){
  /*
    writes data to file
  */
  if(write(treasure_file_desc,&treasure,sizeof(treasure_t))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(170);
  }
}

int read_treasure_from_stdin(treasure_t *treasure,char *hunt_id){
  /*
    Reads treasure input
    Return 1 if all inputs are valid and 0 otherwise
  */
  char buffer[20],*endptr=NULL;
  
  printf("Treasure ID: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;

  //checks if input is valid
  treasure->treasure_id=(int)strtol(buffer,&endptr,10);
  if(*endptr!='\n' || endptr==buffer) return 0;

  endptr=NULL;
  printf("Value: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->value=(int)strtol(buffer,&endptr,10);
  if(*endptr!='\n' || endptr==buffer) return 0;
  
  endptr=NULL;
  printf("User Name: ");
  if(fgets(treasure->user_name,NAME_LENGTH,stdin)==NULL || strlen(treasure->user_name)==1) return 0;
  treasure->user_name[strlen(treasure->user_name)-1]='\0';

  endptr=NULL;
  printf("Clue: ");
  if(fgets(treasure->clue,CLUE_LENGTH,stdin)==NULL || strlen(treasure->clue)==1) return 0;
  treasure->clue[strlen(treasure->clue)-1]='\0';

  endptr=NULL;
  printf("Latitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->latitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer) return 0;

  endptr=NULL;
  printf("Longitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->longitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer) return 0;

  //gets absolute path of the treasures file
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  strcat(path,"/");
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  int treasure_file_desc;
  treasure_t temp;
  if((treasure_file_desc=open(path,O_RDONLY))!=-1){
    //if file exists, checks for no treasure_id - user_name duplicates
    while(read_treasure_from_file(treasure_file_desc,&temp)){
      if(temp.treasure_id==treasure->treasure_id && strcmp(temp.user_name,treasure->user_name)==0) return 0;
    }
    return 1;
  }
  //if file doesn't exist, there are no duplicates
  else if(errno==ENOENT) return 1;
  
  //if it encountered any other error, exits
  perror(NULL);
  exit(-1);
}

void write_treasure_to_stdin(treasure_t treasure){
  printf("Treasure ID: %d\n",treasure.treasure_id);
  printf("Value: %d\n",treasure.value);
  printf("User Name: %s\n",treasure.user_name);
  printf("Clue: %s\n",treasure.clue);
  printf("Latitude: %f\n",treasure.latitude);
  printf("Longitude: %f\n\n",treasure.longitude);
}

void log_operation(int log_file_desc,char *message){
  /*
    takes the time at which the operation is performed
  */
  time_t time_of_operation;
  if((time_of_operation=time(NULL))==-1){
    perror(NULL);
    exit(-1);
  }
  /*
    converts it to calendar time and write to log file
  */
  char *calendar_time=ctime(&time_of_operation);
  if(write(log_file_desc,calendar_time,strlen(calendar_time))==-1){
    perror(NULL);
    exit(-1);
  }
  /*
    writes message to log file
  */
  if(write(log_file_desc,message,strlen(message))==-1){
    perror(NULL);
    exit(-1);
  }
}

void log_add(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[120];
  sprintf(message,"Added treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void log_list(int log_file_desc){
  /*
    creates and appropriate message and calls log_operation
  */
  log_operation(log_file_desc,"Listed hunt\n\n");
}

void log_view(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[120];
  sprintf(message,"Viewed treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void log_remove_treasure(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[120];
  sprintf(message,"Removed treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void add(char hunt_id[]){
  /*
    reads treasure data
  */
  treasure_t treasure;
  int valid;
  do{
    valid=read_treasure_from_stdin(&treasure,hunt_id);
    if(!valid) printf("INVALID INPUT\n");
  }while(valid==0);
  /*
    gets the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(11);
  }
  /*
    opens program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(12);
  }
  /*
    creates the path for the treasures file
  */
  strcat(path,"/");
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  /*
    parses through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    /*
      checks if the current file is a directory and
      if it has the same as hunt_id parameter
    */
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	creates path for log file
      */
      strcpy(strrchr(path,'/'),"/logged_hunt.txt");
      /*
	opens log file
      */
      int log_file_desc;
      if((log_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(132);
      }
      /*
	writes data to file
      */
      write_treasure_to_file(treasure_file_desc,treasure);
      /*
	logs operation
      */
      log_add(log_file_desc,treasure);
      /*
	closes files
      */
      if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
	perror(NULL);
	exit(141);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(15);
  }
  /*
    if theres no such directory, create one
  */
  if(mkdir(hunt_id,0755)!=0){
    perror(NULL);
    exit(16);
  }
  /*
    creates and opens file
  */
  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_CREAT|O_WRONLY,0644))==-1){
    perror(NULL);
    exit(132);
  }
  /*
    creates path for log file
  */
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
  /*
    creates and opens log file
  */
  int log_file_desc;
  if((log_file_desc=open(path,O_CREAT|O_WRONLY,0644))==-1){
    perror(NULL);
    exit(132);
  }
  /*
    creates symbolic link to log file
  */
  char link_path[PATH_MAX]="logged_hunt-";
  strcat(link_path,hunt_id);
  if(symlink(path,link_path)==-1){
    printf("%s\n%s\n",path,link_path);
    perror(NULL);
    exit(-1);
  }
  /*
    wrote data to file
  */
  write_treasure_to_file(treasure_file_desc,treasure);
  /*
    logs operation
  */
  log_add(log_file_desc,treasure);
  /*
    closes files
  */
  if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
    perror(NULL);
    exit(142);
  }
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void list(char hunt_id[]){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the hunt directory
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      /*
	gets status info for hunt directory
      */
      struct stat hunt_stat;
      if(stat(path,&hunt_stat)==-1){
	perror(NULL);
	exit(20);
      }
      /*
	creates the path fot the treasures file
      */
      strcat(path,"/treasures.bin");
      /*
	gets status info for treasures file
      */
      struct stat treasures_stat;
      if(stat(path,&treasures_stat)==-1){
	perror(NULL);
	exit(20);
      }
      /*
	prints time of last modification of the treasures file
      */
      printf("Name: %s\n",hunt_id);
      printf("Total size: %ld\n",hunt_stat.st_size);
      printf("Treasures file last modification time: %s===============================\n",ctime(&(treasures_stat.st_mtim.tv_sec)));
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDONLY))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	parses through the treasures file
	and prints to stdout its content
      */
      treasure_t treasure;
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	write_treasure_to_stdin(treasure);
      }
      /*
	creates path for log file
      */
      strcpy(strrchr(path,'/'),"/logged_hunt.txt");
      /*
	opens log file
      */
      int log_file_desc;
      if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(132);
      }
      /*
	logs operation
      */
      log_list(log_file_desc);
      /*
	closes files
      */
      if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
	perror(NULL);
	exit(142);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void view(char hunt_id[],int treasure_id){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the treasures file
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      strcat(path,"/treasures.bin");
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDONLY))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	creates path for log file
      */
      strcpy(strrchr(path,'/'),"/logged_hunt.txt");
      /*
	opens log file
      */
      int log_file_desc;
      if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(132);
      }
      /*
	parses through the treasures and
	prints the treasure with treasure_id
	equal to the one received through
	treasure_id parameter
      */
      treasure_t treasure;
      int found=0;
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	if(treasure.treasure_id==treasure_id){
	  found=1;
	  write_treasure_to_stdin(treasure);
	  /*
	    logs operation
	  */
	  log_view(log_file_desc,treasure);
	}
      }
      if(!found) printf("Treasure with id %d does not exist\n",treasure_id);
      /*
	closes files and directory
      */
      if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0 || closedir(current_dir)==-1){
	perror(NULL);
	exit(142);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void remove_treasure(char hunt_id[],int treasure_id){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the treasures file
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      strcat(path,"/treasures.bin");
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDWR))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	parses through the treasures and
	prints the treasure with treasure_id
	equal to the one received through
	treasure_id parameter
      */
      int bytes_read=0,found=0;
      off_t write_pos,read_pos;
      treasure_t treasure,log_treasure;
      /*
	write_pos containts the offset at which we can write
      */
      write_pos=lseek(treasure_file_desc,0,SEEK_SET);
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	if(treasure.treasure_id!=treasure_id){
	  bytes_read+=sizeof(treasure_t);
	  /*
	    read_pos retains the offset at which we can read
	  */
	  read_pos=lseek(treasure_file_desc,0,SEEK_CUR);
	  /*
	    writes at write_pos
	  */
	  lseek(treasure_file_desc,write_pos,SEEK_SET);
	  write_treasure_to_file(treasure_file_desc,treasure);
	  /*
	    updates write_pos
	  */
	  write_pos=lseek(treasure_file_desc,0,SEEK_CUR);
	  /*
	    returns to read_pos for the next read
	  */
	  lseek(treasure_file_desc,read_pos,SEEK_SET);
	}
	else if(!found){
	  found=1;
	  log_treasure=treasure;
	}
	/*
	if(found){
	  
	    //overwrites the previous treasure with the current one
	    //(in the long run it eliminates the found treasure)
	  
	  lseek(treasure_file_desc,-2*sizeof(treasure_t),SEEK_CUR);
	  write_treasure_to_file(treasure_file_desc,treasure);
	  lseek(treasure_file_desc,sizeof(treasure_t),SEEK_CUR);
	}

	bytes_read+=sizeof(treasure_t);
	
	  //checks if it found the treasure
	
	if(treasure_id==treasure.treasure_id && !found){
	  found=1;
	  log_treasure=treasure;
	}
	*/
      }
      if(found){
	ftruncate(treasure_file_desc,bytes_read);
	/*
	  closes file
	*/
	if(close(treasure_file_desc)!=0){
	  perror(NULL);
	  exit(142);
	}
	/*
	  creates path for log file
	*/
	strcpy(strrchr(path,'/'),"/logged_hunt.txt");
	/*
	  opens log file
	*/
	int log_file_desc;
	if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	  perror(NULL);
	  exit(132);
	}
	/*
	  logs operation
	*/
	log_remove_treasure(log_file_desc,log_treasure);
	/*
	  closes log file
	*/
	if(close(log_file_desc)!=0){
	  perror(NULL);
	  exit(142);
	}
	/*
	  closes directory
	*/
	if(closedir(current_dir)==-1){
	  perror(NULL);
	  exit(-1);
	}
	return;
      }
      printf("Treasure with id %d does not exist\n",treasure_id);
      /*
	closes file
      */
      if(close(treasure_file_desc)!=0){
	perror(NULL);
	exit(142);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void remove_hunt(char hunt_id[]){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through program directory
  */
  struct dirent *current_dir_entry;
  errno=0;
  while((current_dir_entry=readdir(current_dir))!=NULL){
    /*
      checks if the current file is a directory and
      if it has the same as hunt_id parameter
    */
    if(current_dir_entry->d_type==DT_DIR && strcmp(current_dir_entry->d_name,hunt_id)==0){
      /*
	getting symlinks path
      */
      strcat(path,"/logged_hunt-");
      strcat(path,hunt_id);
      /*
	removing symlink
      */
      if(remove(path)==-1){
	perror(NULL);
	exit(-1);
      }
      /*
	getting hunt directory path
      */
      strcpy(strrchr(path,'/')+1,hunt_id);
      /*
	opening hunt directory
      */
      DIR *hunt_dir;
      if((hunt_dir=opendir(path))==NULL){
	perror(NULL);
	exit(22);
      }
      /*
	parsing through hunt directory
      */
      struct dirent *hunt_dir_entry;
      errno=0;
      while((hunt_dir_entry=readdir(hunt_dir))!=NULL){
	/*
	  getting the path for each file in directory and removing it
	*/
	if(strcmp(hunt_dir_entry->d_name,".")!=0 && strcmp(hunt_dir_entry->d_name,"..")!=0){
	  char file_path[PATH_MAX];
	  strcpy(file_path,path);
	  strcat(file_path,"/");
	  strcat(file_path,hunt_dir_entry->d_name);
	  if(remove(file_path)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
      }
      if(errno){
	perror(NULL);
	exit(25);
      }
      /*
	closes hunt directory
      */
      if(closedir(hunt_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      /*
	removing direcotry
      */
      if(remove(path)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes program directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

int main(int argc,char **argv){
  if(argc==2 && strcmp(argv[1],"--help")==0){
    print_usage_info();
  }
  else if(argc==3 && strcmp(argv[1],"--add")==0){
    add(argv[2]);
  }
  else if(argc==3 && strcmp(argv[1],"--list")==0){
    list(argv[2]);
  }
  else if(argc==4 && strcmp(argv[1],"--view")==0){
    int treasure_id;
    char *endptr=NULL;
    /*
      checks if the received treasure id is valid
    */
    treasure_id=(int)strtol(argv[3],&endptr,10);
    if(*endptr!='\0'){
      printf("Invalid treasue ID\n");
      return 0;
    }
    view(argv[2],treasure_id);
  }
  else if(argc==4 && strcmp(argv[1],"--remove_treasure")==0){
    int treasure_id;
    char *endptr=NULL;
    /*
      checks if the received treasure id is valid
    */
    treasure_id=(int)strtol(argv[3],&endptr,10);
    if(*endptr!='\0'){
      printf("Invalid treasue ID\n");
      return 0;
    }
    remove_treasure(argv[2],treasure_id);
  }
  else if(argc==3 && strcmp(argv[1],"--remove_hunt")==0){
    remove_hunt(argv[2]);
  }
  else{
    printf("Incorrect format\n");
    exit(-1);
  }
  return 0;
}
