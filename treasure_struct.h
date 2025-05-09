#ifndef TREASURE_STRUCT
#define TREASURE_STRUCT

#define NAME_LENGTH 50
#define CLUE_LENGTH 100

typedef struct{
  int treasure_id,value;
  float latitude,longitude;
  char user_name[NAME_LENGTH],clue[CLUE_LENGTH];
}treasure_t;

#endif
