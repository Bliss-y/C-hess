#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// basic linked list

struct st_ch_hashelement
{
    char *key;
    int keylen;
    int vlen;
    st_ch_hashelement *next;
    char *value;
};

struct st_ch_hashmap
{
    st_ch_hashelement **table;
    int filled_len;
    int tablelen;
};

int ch_worst_hash(int max, const char *value, int len);
st_ch_hashmap *ch_create_hash_map();
st_ch_hashelement *ch_hash_get(st_ch_hashmap *map, const char *key, int len);
int ch_destroy_hash(st_ch_hashmap *map);
int ch_hashmap_insert(st_ch_hashmap *map, const char *key, int keylen, const char *value, int valuelen);