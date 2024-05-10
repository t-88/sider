#ifndef HMAP_H
#define HMAP_H

#include <stdint.h>
#include <assert.h>

typedef struct HNode HNode;
typedef struct HNode {
  HNode* next;
  uint64_t hkey;
  char* key;
  char* val;
} HNode;


typedef struct HTable {
  HNode** items;
  uint32_t capacity;
  uint32_t size;
} HTable;

typedef struct HMap {
  HTable* table;
} HMap;

static HNode* hn_detach(HNode** n);

static HTable* ht_init(int32_t size);
static void ht_free(HTable* t);
static HNode* ht_create_node(char* key, char* val);
static void ht_insert(HTable* t, HNode* n);

HMap* hm_init();
void hm_free(HMap* m);
void hm_print(HMap* m);
void hm_insert(HMap* m, char* key,char* val);
void hm_resize(HMap* m);
char* hm_get(HMap* m,char* key);

#endif // HMAP_H


#ifdef HMAP_IMPLEMENTATION


// hashing algorithm  http://www.cse.yorku.ca/~oz/hash.html
static uint64_t hash_djb2(const char *str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}


// hash node functions
// --------------------
static HNode* hn_detach(HNode** n) {
  HNode* node = *n;
  *n = node->next;
  return node;
}
// --------------------

// hash table functions
// --------------------
static HTable* ht_init(int32_t size) {
  assert(size > 0 && ((size - 1) & size) == 0);
  HTable* t = malloc(sizeof(*t));
  t->items = calloc(sizeof(t->items[0]) , size);
  t->capacity = size;
  t->size = 0;
  return t;
}

static void ht_free(HTable* t) {
  if(!t) return;

  for(int32_t i = 0; i < t->capacity; i++) {
    if(!t->items[i]) continue;

    HNode* cur = t->items[i];
    while(cur != NULL) {
      HNode* next = cur->next;
      free(cur->key);
      free(cur->val);
      free(cur);
      cur = next;
    }
  }
}

static HNode* ht_create_node(char* key, char* val) {
  HNode* n = malloc(sizeof(*n));
  n->key = malloc(strlen(key));
  n->val = malloc(strlen(key));
  strcpy(n->key,key);
  strcpy(n->val,val);
  n->hkey = hash_djb2(n->key);
  return n;
}

static void ht_insert(HTable* t, HNode* n) {
  uint32_t pos = n->hkey & (t->capacity - 1);
  
  HNode* cur = t->items[pos];
  while(cur != NULL) { 
    if(cur->hkey == n->hkey && strcmp(cur->key,n->key) == 0) {
      char* val = n->val;
      free(n->key);
      free(n);
      free(cur->val);
      cur->val = n->val;
      return;
    }
    cur = cur->next;
  }
  
  HNode* head = t->items[pos];
  n->next = head;
  t->items[pos] = n;

  t->size++;
}
// --------------------


// hash map functions
// -------------------
HMap* hm_init() {
  HMap* m = malloc(sizeof(*m));
  m->table = ht_init(4);
  return m;
}

void hm_free(HMap* m) {
  if(!m) return;
  ht_free(m->table);
  free(m);
} 

void hm_print(HMap* m) {
  HTable* t = m->table;

  printf("{\n");
  for(int32_t i = 0; i < t->capacity; i++) {
    if(!t->items[i]) continue;
    
    HNode* cur = t->items[i];
    while(cur != NULL) {
      printf("  \"%s\" : \"%s\",\n",cur->key,cur->val);
      cur = cur->next;
    }
  }
  printf("}\n");
}

void hm_insert(HMap* m, char* key,char* val) {
  HNode* n = ht_create_node(key,val);
  ht_insert(m->table,n); 
  
  float load_factor = (float) m->table->size / m->table->capacity;
  if(load_factor > 0.5) {
    hm_resize(m); 
  }

}

void hm_resize(HMap* m) {
  HTable* t = m->table;
  m->table = ht_init(m->table->capacity * 2);

  for(int32_t i = 0; i < t->capacity; i++) {
    if(!t->items[i]) continue;

    HNode** addr = &t->items[i];
    while(*addr != NULL) 
      ht_insert(m->table, hn_detach(addr)); 
  }
  free(t);
}

char* hm_get(HMap* m, char* key) {
  HTable* t = m->table;
  uint64_t hkey = hash_djb2(key);
  uint32_t pos = hkey & (t->capacity - 1);

  HNode* cur = t->items[pos];
  while(cur != NULL) { 
    if(cur->hkey == hkey && strcmp(cur->key,key) == 0) {
      return cur->val;
    }
    cur = cur->next;
  }

  return NULL;
}
// -------------------

#endif
