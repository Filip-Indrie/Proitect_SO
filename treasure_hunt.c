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
  /*
    prnts usage info
  */
  printf("Usage: ./treasure_hunt <operation> <arg1> <arg2> ...\n");
  printf("Operations:\n");
  printf("\t--add <hunt_id> --> adds a new treasure to the specified hunt\n");
  printf("\t--list <hunt_id> --> lists all treasures in the specified hunt\n");
  printf("\t--list_hunts <hunt_id> --> lists all hunts and the total number of treasures in the respective hunt\n");
  printf("\t--view <hunt_id> <treasure_id> <user_name> --> lists the details of all treasures with the specified ID and User Name in the specified hunt. If either ID or User Name is not specified, it lists all treasures that have their respective field mathcing wtih the specfied argument. If none are specfed, it lists all the treasures in the specified hunt\n");
  printf("\t--remove_treasure <hunt_id> <treasure_id> <user_name> --> removes the treasures with the specified ID and User Name from the specified hunt. (ID or User Name may not be specified (one at a time); in that case, it removes all treasures that have their respective field mathcing wtih the specfied argument)\n");
  printf("\t--remove_hunt <hunt_id> --> removes the specified hunt\n");
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

int read_treasure_from_file(int treasure_file_desc,treasure_t *treasure){
  /*
    returns 1 if all data was read and
    0 if read data is incomplete or
    an error occurred
  */
  int bytes_read;
  if((bytes_read=read(treasure_file_desc,treasure,sizeof(treasure_t)))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(-1);
    }
    exit(-1);
  }
  if(bytes_read!=sizeof(treasure_t)) return 0;
  return 1;
}

void write_treasure_to_file(int treasure_file_desc,treasure_t treasure){
  /*
    writes treasure to file
  */
  if(write(treasure_file_desc,&treasure,sizeof(treasure_t))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(-1);
    }
    exit(-1);
  }
}

int read_treasure_from_stdin(treasure_t *treasure,char *hunt_id){
  /*
    Reads treasure from stdin
    Returns 1 if all inputs are valid and 0 otherwise
  */
  char buffer[20],*endptr=NULL;
  printf("Treasure ID: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;

  //checks if input is valid
  treasure->treasure_id=(int)strtol(buffer,&endptr,10);
  if(*endptr!='\n' || endptr==buffer){
    if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
    return 0;
  }

  endptr=NULL;
  printf("Value: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->value=(int)strtol(buffer,&endptr,10);
  if(*endptr!='\n' || endptr==buffer){
    if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
    return 0;
  }
  
  printf("User Name: ");
  if(fgets(treasure->user_name,NAME_LENGTH,stdin)==NULL || strlen(treasure->user_name)==1) return 0;
  if(treasure->user_name[strlen(treasure->user_name)-1]!='\n') clear_buffer();
  treasure->user_name[strlen(treasure->user_name)-1]='\0';

  printf("Clue: ");
  if(fgets(treasure->clue,CLUE_LENGTH,stdin)==NULL || strlen(treasure->clue)==1) return 0;
  if(treasure->clue[strlen(treasure->clue)-1]!='\n') clear_buffer();
  treasure->clue[strlen(treasure->clue)-1]='\0';

  endptr=NULL;
  printf("Latitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->latitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer){
    if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
    return 0;
  }

  endptr=NULL;
  printf("Longitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->longitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer){
    if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
    return 0;
  }

  //gets  path of the treasures file
  char path[PATH_MAX]="";
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  int treasure_file_desc;
  treasure_t temp;
  if((treasure_file_desc=open(path,O_RDONLY))!=-1){
    //if file exists, checks for treasure_id - user_name duplicates
    
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
  /*
    writes treasure to stdin
  */
  printf("Treasure ID: %d\n",treasure.treasure_id);
  printf("Value: %d\n",treasure.value);
  printf("User Name: %s\n",treasure.user_name);
  printf("Clue: %s\n",treasure.clue);
  printf("Latitude: %f\n",treasure.latitude);
  printf("Longitude: %f\n\n",treasure.longitude);
}

void log_operation(int log_file_desc,char *message){
  /*
    logs the time and message recevived in the log file
  */
  time_t time_of_operation;
  if((time_of_operation=time(NULL))==-1){
    perror(NULL);
    exit(-1);
  }
  char *calendar_time=ctime(&time_of_operation);
  if(write(log_file_desc,calendar_time,strlen(calendar_time))==-1){
    perror(NULL);
    exit(-1);
  }
  if(write(log_file_desc,message,strlen(message))==-1){
    perror(NULL);
    exit(-1);
  }
}

void log_add(int log_file_desc,treasure_t treasure){
  /*
    creates 'add' message and calls log_operation
  */
  char message[120];
  sprintf(message,"Added treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void log_list(int log_file_desc){
  /*
    creates 'list' message and calls log_operation
  */
  log_operation(log_file_desc,"Listed hunt\n\n");
}

void log_view(int log_file_desc,treasure_t treasure){
  /*
    creates 'view' message and calls log_operation
  */
  char message[120];
  sprintf(message,"Viewed treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void log_remove_treasure(int log_file_desc,treasure_t treasure){
  /*
    creates 'remove_treasure' message and calls log_operation
  */
  char message[120];
  sprintf(message,"Removed treasure with ID: %d and User Name: %s\n\n",treasure.treasure_id,treasure.user_name);
  log_operation(log_file_desc,message);
}

void add(char hunt_id[]){
  /*
    reads a treasure from stdin and adds it to treasures file of the hunts directory
    if there's no such directory, creates one and adds to it
  */

  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0 && errno!=ENOENT){
    perror(NULL);
    exit(-1);
  }
  else if(stat_return_val==0 && !S_ISDIR(dir_stat.st_mode)){
    printf("There already exists a file, that's not a directory, with the same name\n");
    return;
  }
  
  treasure_t treasure;
  int valid;
  do{
    valid=read_treasure_from_stdin(&treasure,hunt_id);
    if(!valid) printf("INVALID INPUT\n");
  }while(valid==0);
  
  if(stat_return_val!=0 && errno==ENOENT){
    //if the directory doesn't exist
    
    if(mkdir(hunt_id,0755)!=0){
      perror(NULL);
      exit(-1);
    }
  }

  //creates the path for the treasures file
  char path[PATH_MAX]="";
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
    perror(NULL);
    exit(-1);
  }
  
  //creates path for log file
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
  int log_file_desc;
  if((log_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
    perror(NULL);
    exit(-1);
  }

  if(stat_return_val!=0 && errno==ENOENT){
    //if the directory doesn't exist
    
    // creates symbolic link to log file
    char link_path[PATH_MAX]="logged_hunt-";
    strcat(link_path,hunt_id);
    if(symlink(path,link_path)==-1){
      perror(NULL);
      exit(-1);
    }
  }
  
  write_treasure_to_file(treasure_file_desc,treasure);
  log_add(log_file_desc,treasure);
  if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
    perror(NULL);
    exit(-1);
  }
}

void list(char hunt_id[]){
  /*
    lists all the treasures in the specified hunt
  */
  
  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0){
    perror(NULL);
    exit(-1);
  }
  else if(!S_ISDIR(dir_stat.st_mode)){
    printf("Provided file is not a directory\n");
    return;
  }
      
  //creates the path for the treasures file
  char path[PATH_MAX]="";
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
      
  //gets status info for treasures file
  struct stat treasures_stat;
  if(stat(path,&treasures_stat)==-1){
    perror(NULL);
    exit(-1);
  }

  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_RDONLY))==-1){
    perror(NULL);
    exit(-1);
  }
      
  //creates the path fot the log file
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");

  int log_file_desc;
  if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
    perror(NULL);
    exit(-1);
  }
  log_list(log_file_desc);

  DIR *hunt_dir;
  if((hunt_dir=opendir(hunt_id))==NULL){
    perror(NULL);
    exit(-1);
  }
  
  //sums the sizes of all the files in the hunt directory
  long int total_size=0;
  struct dirent *hunt_dir_entry;
  errno=0;
  while((hunt_dir_entry=readdir(hunt_dir))!=NULL){
    if(strcmp(hunt_dir_entry->d_name,".")!=0 && strcmp(hunt_dir_entry->d_name,"..")!=0){
      strcpy(strrchr(path,'/')+1,hunt_dir_entry->d_name);
      struct stat file_stat;
      if(stat(path,&file_stat)==-1){
	perror(NULL);
	exit(-1);
      }
      total_size+=file_stat.st_size;
    }
  }
  if(errno){
    perror(NULL);
    exit(-1);
  }
      
  printf("Name: %s\n",hunt_id);
  printf("Total size: %ld\n",total_size);
  printf("Treasures file last modification time: %s===============================\n",ctime(&(treasures_stat.st_mtim.tv_sec)));
      
  //parses through the treasures file and prints to stdout its content
  treasure_t treasure;
  while(read_treasure_from_file(treasure_file_desc,&treasure)){
    write_treasure_to_stdin(treasure);
  }
      
  if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0 || closedir(hunt_dir)==-1){
    perror(NULL);
    exit(-1);
  }
  return;
}

void list_hunts(){
  /*
    lists all hunts and the number of treasures in each one
  */
  int found=0;
  
  DIR *current_dir;
  if((current_dir=opendir("."))==NULL){
    perror(NULL);
    exit(-1);
  }
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
      else if(S_ISDIR(entry_stat.st_mode)){
	char treasures_file_path[PATH_MAX]="./";
	strcat(treasures_file_path,current_dir_entry->d_name);
	strcat(treasures_file_path,"/treasures.bin");
	struct stat treasures_file_stat;
	int treasures_file_stat_return_val=stat(treasures_file_path,&treasures_file_stat);
	if(treasures_file_stat_return_val==0 && S_ISREG(treasures_file_stat.st_mode)){
	  found=1;
	  printf("%s: ",current_dir_entry->d_name);
	  if(treasures_file_stat.st_size%sizeof(treasure_t)==0){
	    printf("%ld treasure(s)\n",treasures_file_stat.st_size/sizeof(treasure_t));
	  }
	  else{
	    printf("treasures file corrupted\n");
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
  return;
}

void view(char hunt_id[],int *treasure_id,char *user_name){
  /*
    if both treasure_id and user_name are not NULL, lists all treasures with the respective user_name and treasure_id
    if only treasure_id in not NULL, lists all treasures with the respective treasure_id
    if only user_name is not NULL, lists all treasures with the respective user_name
  */

  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0){
    perror(NULL);
    exit(-1);
  }
  else if(!S_ISDIR(dir_stat.st_mode)){
    printf("Provided file is not a directory\n");
    return;
  }

  //creates path for treasures file
  char path[PATH_MAX]="";
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_RDONLY))==-1){
    perror(NULL);
    exit(-1);
  }
      
  //creates path for log file
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
  int log_file_desc;
  if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
    perror(NULL);
    exit(-1);
  }
  treasure_t treasure;
  int found=0;
  while(read_treasure_from_file(treasure_file_desc,&treasure)){
    if(treasure_id!=NULL && user_name!=NULL){
      if(treasure.treasure_id==*treasure_id && strcmp(user_name,treasure.user_name)==0){
	write_treasure_to_stdin(treasure);
	log_view(log_file_desc,treasure);
	if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
	  perror(NULL);
	  exit(-1);
	}
	return;
      }
    }
    else if(treasure_id!=NULL){
      if(treasure.treasure_id==*treasure_id){
	found=1;
	write_treasure_to_stdin(treasure);
	log_view(log_file_desc,treasure);
      }
    }
    else{
      if(strcmp(user_name,treasure.user_name)==0){
	found=1;
	write_treasure_to_stdin(treasure);
	log_view(log_file_desc,treasure);
      }
    }
  }
  if(!found) printf("Couldn't find any treasure(s)\n");
  if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
    perror(NULL);
    exit(-1);
  }
}

void remove_treasure(char hunt_id[],int *treasure_id,char *user_name){
  /*
    if both treasure_id and user_name are not NULL, removes all treasures with the respective user_name and treasure_id
    if only treasure_id in not NULL, removes all treasures with the respective treasure_id
    if only user_name is not NULL, removes all treasures with the respective user_name
  */
  
  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0){
    perror(NULL);
    exit(-1);
  }
  else if(!S_ISDIR(dir_stat.st_mode)){
    printf("Provided file is not a directory\n");
    return;
  }
      
  //creates the path for the treasures file
  char path[PATH_MAX]="";
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_RDWR))==-1){
    perror(NULL);
    exit(-1);
  }
      
  //creates path for log file
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
  int log_file_desc;
  if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
    perror(NULL);
    exit(-1);
  }
      
  int bytes_read=0,found=0;
  off_t write_pos,read_pos;
  treasure_t treasure;
      
  //write_pos containts the offset at which we can write
  write_pos=lseek(treasure_file_desc,0,SEEK_SET);
  while(read_treasure_from_file(treasure_file_desc,&treasure)){
    if(treasure_id!=NULL && user_name!=NULL){
      if(treasure.treasure_id!=(*treasure_id) || strcmp(user_name,treasure.user_name)!=0){
	bytes_read+=sizeof(treasure_t);
	  
	//read_pos retains the offset at which we can read
	if((read_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	  
	if(lseek(treasure_file_desc,write_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
	write_treasure_to_file(treasure_file_desc,treasure);
	if((write_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	  
	// returns cursor to read_pos for the next read
	if(lseek(treasure_file_desc,read_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      else{
	found=1;
	log_remove_treasure(log_file_desc,treasure);
      }
    }
    else if(treasure_id!=NULL){
      if(treasure.treasure_id!=*treasure_id){
	bytes_read+=sizeof(treasure_t);
	if((read_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(lseek(treasure_file_desc,write_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
	write_treasure_to_file(treasure_file_desc,treasure);
	if((write_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(lseek(treasure_file_desc,read_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      else{
	found=1;
	log_remove_treasure(log_file_desc,treasure);
      }
    }
    else{
      if(strcmp(user_name,treasure.user_name)!=0){
	bytes_read+=sizeof(treasure_t);
	if((read_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(lseek(treasure_file_desc,write_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
	write_treasure_to_file(treasure_file_desc,treasure);
	if((write_pos=lseek(treasure_file_desc,0,SEEK_CUR))==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(lseek(treasure_file_desc,read_pos,SEEK_SET)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      else{
	found=1;
	log_remove_treasure(log_file_desc,treasure);
      }
    }
  }
  if(!found) printf("Couldn't find any treasure(s)\n");
  if(ftruncate(treasure_file_desc,bytes_read)==-1 || close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
    perror(NULL);
    exit(-1);
  }
}

void remove_hunt(char hunt_id[]){
  /*
    removes a hunt with all its associated files
  */
  
  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0){
    perror(NULL);
    exit(-1);
  }
  else if(!S_ISDIR(dir_stat.st_mode)){
    printf("Provided file is not a directory\n");
    return;
  }

  //removing symlink
  char path[PATH_MAX]="logged_hunt-";
  strcat(path,hunt_id);
  if(remove(path)==-1){
    perror(NULL);
    exit(-1);
  }
      
  //getting hunt directory path
  strcpy(path,hunt_id);
  DIR *hunt_dir;
  if((hunt_dir=opendir(path))==NULL){
    perror(NULL);
    exit(-1);
  }
  struct dirent *hunt_dir_entry;
  errno=0;
  while((hunt_dir_entry=readdir(hunt_dir))!=NULL){
	
    //getting the path for each file in directory and removing it
    if(strcmp(hunt_dir_entry->d_name,".")!=0 && strcmp(hunt_dir_entry->d_name,"..")!=0){
      char file_path[PATH_MAX];
      strcpy(file_path,hunt_id);
      strcat(file_path,"/");
      strcat(file_path,hunt_dir_entry->d_name);
      if(remove(file_path)==-1){
	perror(NULL);
	exit(-1);
      }
    }
  }
  if(errno || closedir(hunt_dir)==-1 || remove(hunt_id)==-1){
    perror(NULL);
    exit(-1);
  }
  return;
}

void print_log(char hunt_id[]){
  /*
    prints the log file of the specified Hunt ID to stdout
  */

  struct stat dir_stat;
  int stat_return_val=stat(hunt_id,&dir_stat);
  if(stat_return_val!=0){
    perror(NULL);
    exit(-1);
  }
  else if(!S_ISDIR(dir_stat.st_mode)){
    printf("Provided file is not a directory\n");
    return;
  }
  
  //getting the path of hunts log file symlink
  char path[PATH_MAX]="logged_hunt-";
  strcat(path,hunt_id);

  int log_file_desc;
  if((log_file_desc=open(path,O_RDONLY))!=-1){
    char buffer[101];
    int bytes_read;
    while((bytes_read=read(log_file_desc,buffer,100))>0){
      buffer[bytes_read]='\0';
      printf("%s",buffer);
    }
    if(bytes_read==-1 || close(log_file_desc)!=0){
      perror(NULL);
      exit(-1);
    }
  }
  else{
    perror(NULL);
    exit(-1);
  }
}

int main(int argc,char **argv){
  if(argc==1){
    printf("Incorrect format\n");
    return 0;
  }
  else if(strcmp(argv[1],"--log")==0 && argc==3){
    print_log(argv[2]);
  }
  else if(strcmp(argv[1],"--help")==0 && argc==2){
    print_usage_info();
  }
  else if(strcmp(argv[1],"--add")==0 && argc==3){
    add(argv[2]);
  }
  else if(strcmp(argv[1],"--list")==0 && argc==3){
    list(argv[2]);
  }
  else if(strcmp(argv[1],"--list_hunts")==0 && argc==2){
    list_hunts();
  }
  else if(strcmp(argv[1],"--view")==0){
    int treasure_id;
    char *endptr=NULL;
    if(argc==5){
      
      //checks if the received treasure id is valid
      treasure_id=(int)strtol(argv[3],&endptr,10);
      if(*endptr!='\0'){
        printf("Invalid treasue ID\n");
        exit(-1);
      }
      if(strlen(argv[4])>=NAME_LENGTH){
	printf("Invalid User Name\n");
	exit(-1);
      }
      view(argv[2],&treasure_id,argv[4]);
    }
    else if(argc==4){
      
      //checks if the received argument is an int
      treasure_id=(int)strtol(argv[3],&endptr,10);
      if(*endptr!='\0'){
	//if it can't be converted to an int, its considered to be the user_name
	
	if(strlen(argv[3])>=NAME_LENGTH){
	  printf("Invalid argument\n");
	  exit(-1);
	}
	view(argv[2],NULL,argv[3]);
      }
      else{
	view(argv[2],&treasure_id,NULL);
      }
    }
    else if(argc==3){
      list(argv[2]);
    }
    else{
      printf("Incorrect format\n");
      exit(-1);
    }
  }
  else if(strcmp(argv[1],"--remove_treasure")==0){
    int treasure_id;
    char *endptr=NULL;
    if(argc==5){
      
      //checks if the received treasure id is valid
      treasure_id=(int)strtol(argv[3],&endptr,10);
      if(*endptr!='\0'){
        printf("Invalid treasue ID\n");
        exit(-1);
      }
      if(strlen(argv[4])>=NAME_LENGTH){
	printf("Invalid User Name\n");
	exit(-1);
      }
      remove_treasure(argv[2],&treasure_id,argv[4]);
    }
    else if(argc==4){
      
      //checks if the received argument is an int
      treasure_id=(int)strtol(argv[3],&endptr,10);
      if(*endptr!='\0'){
	//if it can't be converted to an int, its considered to be the user_name
	
	if(strlen(argv[3])>=NAME_LENGTH){
	  printf("Invalid argument\n");
	  exit(-1);
	}
	remove_treasure(argv[2],NULL,argv[3]);
      }
      else{
	remove_treasure(argv[2],&treasure_id,NULL);
      }
    }
    else{
      printf("Incorrect format\n");
      exit(-1);
    }
  }
  else if(strcmp(argv[1],"--remove_hunt")==0 && argc==3){
    remove_hunt(argv[2]);
  }
  else{
    printf("Incorrect format\n");
    exit(-1);
  }
  return 0;
}
