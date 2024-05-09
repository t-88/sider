#ifndef DA_H
#define DA_H

#include <stdlib.h>

// example:
//  #define DA_IMPLEMETATION
//  #include "da.h"
//  
//  define a struct that contains at least capacity , len , items
//  typedef struct DA_string {
//    char** items;
//    uint32_t capacity;
//    uint32_t len;
//  } DA_string;
//
//  int main(void) {
//    DA_string strings = {.capacity = 4}; 
//    da_init(strings);
//
//    da_free(strings);
//  } 


uint32_t next_power_of_two(uint32_t num);

#define da_resize(da,size)                                                     \
{                                                                              \
    (da).capacity = size;                                                      \
    (da).items = realloc(da.items,sizeof(da.items[0]) * (da).capacity);        \
    assert((da).items != NULL && "ERROR: looks like you got no memory");       \
}                                                                              \


#define da_init(da) da_resize((da),(da).capacity)  

#define da_append(da,item)                                                     \
{                                                                              \
  if((da).capacity == 0)                                                       \
  {                                                                            \
    (da).capacity = 1;                                                         \
  }                                                                            \
  if((da).items == NULL) {                                                     \
    da_resize((da),(da).capacity);                                         \
    (da).items = realloc(da.items,sizeof(da.items[0]) * (da).capacity);        \
    assert((da).items != NULL && "ERROR: looks like you got no memory");       \
  } else  if((da).len == (da).capacity - 1)                                    \
  {                                                                            \
    da_resize((da),(da).capacity * 2);                                         \
  }                                                                            \
  (da).items[(da).len] = (item);                                               \
  (da).len += 1;                                                               \
} 

#define da_clear(da)                                                           \
{                                                                              \
  (da).len = 0;                                                                \
}


#define da_free(da)                                                            \
{                                                                              \
  free((da).items);                                                            \
}

#define da_free_items(da)   \
  for(int i = 0; i < (da).len; i++) \
  {\
    if((da).items[i]) \
    {\
      free((da).items[i]);\
    }\
  }


#define da_set(da,idx,item)                                                     \
{                                                                               \
  if((da).capacity <= (idx))                                                      \
  {                                                                             \
    da_resize((da),next_power_of_two((idx) + 1));                                 \
  }                                                                             \
  (da).items[(idx)] = (item);                                                       \
}



#endif // DA_H


#ifdef DA_IMPLEMENTATION

uint32_t next_power_of_two(uint32_t num) 
{
  num--;
  num |= num >> 1;
  num |= num >> 2;
  num |= num >> 4;
  num |= num >> 8;
  num |= num >> 16;
  num++;
  return num;
}

#endif
