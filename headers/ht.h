#ifndef HA_H
#define HA_H

#include <stdint.h>
#include <assert.h>



static uint64_t hash_djb2(const char *str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

typedef struct HT_Node HT_Node;
typedef struct HT_Node {
  HT_Node* next;
  uint32_t hkey;
  char* key;
  char* val;
} HT_Node;


typedef struct HMap {
  HT_Node** items;
  int32_t capacity;
  int32_t size;
} HMap;

typedef struct HTable {
  HMap* map;
} HTable;


void ht_resize(HMap** t);


// count linked elements
static uint32_t htn_count_length(HT_Node* ptr) {
  uint32_t out = 0;
  for(HT_Node* cur = ptr; cur != NULL; cur = cur->next) {
    /* printf("%p\n",cur); */
    out++;
  } 
  return out;
}



void ht_print(HTable* t) 
{
  printf("{\n");
  for(int i = 0; i < t->map->capacity; i++)
    for(HT_Node* cur = t->map->items[i]; cur != NULL; cur = cur->next) 
      printf("  \"%s\" : \"%s\",\n",cur->key,cur->val);
  printf("}\n");
}



HMap* hm_init(int32_t n)
{
  // make sure n > 0 and n is even
  assert(n > 0 && (n - 1) % n); 
  
  HMap* t = (HMap*) malloc(sizeof(sizeof(HMap)));
  t->items = (HT_Node**) calloc(sizeof(t->items[0]), n);
  t->size = 0;
  t->capacity = n;

  return t;
}

HTable* ht_init(int32_t n)
{
  // make sure n > 0 and n is even
  assert(n > 0 && (n - 1) % n); 
  
  HTable* t = (HTable*) malloc(sizeof(HTable));
  t->map = hm_init(n);
  return t;
}

void ht_free(HTable* t) 
{
  if(!t) return;

  for(int i = 0 ; i < t->map->capacity; i++) 
  {
    HT_Node* cur = t->map->items[i];

    while(cur != NULL) 
    {
      HT_Node* next = cur->next;
      free(cur->key);
      free(cur->val);
      free(cur);
      cur = next;
    }
  }
} 


HT_Node** ht_lookup(HMap* t,HT_Node* n) 
{
  // get the position of the node 
  uint32_t pos = n->hkey & (t->capacity - 1);
  HT_Node** addr = &(t->items[pos]);
  for(HT_Node* cur; (cur = *addr) != NULL; addr= &cur->next) 
    if(cur->hkey == n->hkey && strcmp(cur->key,n->key) == 0) 
      return addr;
  return NULL;
}

void ht_insert_node(HMap** t,HT_Node* node) 
{
  // get the position of the node 
  uint32_t pos = node->hkey & ((*t)->capacity - 1);
  
  // look for the node in the list
  // modify it if exsists
  HT_Node** lookup = ht_lookup((*t),node);
  if(lookup != NULL) 
  {
    node->next = (*lookup)->next;
    *lookup = node;
    return;
  }

  // pre-apppend its a new node
  HT_Node* head = (*t)->items[pos];
  node->next = head;
  (*t)->items[pos] = node;
  (*t)->size++;

  float load_factor = (float)(*t)->size / (*t)->capacity;
  if(load_factor > 0.5) 
  {
    ht_resize(t);
  }
}

void ht_insert(HTable* t,char* key, char* val) 
{
  HT_Node* n = malloc(sizeof(*n));

  uint32_t key_len = strlen(key);
  n->key = malloc(key_len);
  strcpy(n->key,key);

  uint32_t val_len = strlen(val);
  n->val = malloc(val_len);
  strcpy(n->val,val);

  n->hkey = hash_djb2(n->key);
  
  ht_insert_node(&t->map,n);
}

HT_Node* ht_detach(HMap* t, HT_Node** addr) 
{
  HT_Node* n = *addr;
  *addr = n->next;
  t->size--;
  return n;
}



void ht_resize(HMap** t) 
{
  HMap* tmp = *t;
  *t = hm_init(tmp->capacity * 2);
  for(int i = 0; i < tmp->capacity; i++)
  {
    while(tmp->items[i]) 
    {
      HT_Node** addr = &(tmp->items[i]);
      HT_Node* detached = ht_detach(tmp,addr);
      ht_insert_node(t,detached);
    }
  }

  free(tmp);
}


#endif // HA_H
