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

// worst hashing function but hey, fuck u
int ch_worst_hash(int max, const char *value, int len)
{
    int sum = 0;
    if (len < 2)
    {
        return -1;
    }
    if (max < 10)
    {
        return -1;
    }
    for (int i = 0; i < len; i++)
    {
        sum += (int)value[i];
    }
    int hash = 0;
    hash = sum % (max - 1);
    return hash;
}

// caller is responsible to free the hashmap
st_ch_hashmap *ch_create_hash_map()
{
    st_ch_hashmap *map = (st_ch_hashmap *)malloc(sizeof(st_ch_hashmap));
    map->table = (st_ch_hashelement **)malloc(sizeof(st_ch_hashelement *) * 16);
    for (int i = 0; i < 16; i++)
    {
        map->table[i] = NULL;
    }
    map->tablelen = 16;
    map->filled_len = 0;
    return map;
}
// find in linked list
st_ch_hashelement *ch_hashelement_find(st_ch_hashelement *e, const char *key, int len)
{
    st_ch_hashelement *head = e;
    while (head)
    {
        if (!strcmp(key, head->key))
        {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

st_ch_hashelement *ch_hash_get(st_ch_hashmap *map, const char *key, int len)
{
    int hash = ch_worst_hash(map->tablelen, key, len);
    if (!map->table[hash])
    {
        return NULL;
    }
    return ch_hashelement_find(map->table[hash], key, len);
}

int ch_rehash(st_ch_hashmap *map)
{
    return 0;
}

int ch_destroy_hash(st_ch_hashmap *map)
{
    int size = map->tablelen;
    for (int i = 0; i < size; i++)
    {
        st_ch_hashelement *e = map->table[i];
        while (e)
        {
            st_ch_hashelement *tmp = e;
            e = tmp->next;
            free(tmp->value);
            free(tmp->key);
            free(tmp);
        }
    }
    free(map->table);
    free(map);
    return 0;
}

void ch_insert_hashelement(st_ch_hashelement **e, int index, const char *key, const int len, const char *value, int valuelen)
{
    st_ch_hashelement *head = e[index];
    if (!head)
    {
        head = (st_ch_hashelement *)malloc(sizeof(st_ch_hashelement));
        head->key = (char *)malloc(len);
        head->keylen = len;
        head->vlen = valuelen;
        head->value = (char *)malloc(valuelen);
        head->next = nullptr;
        strcpy(head->key, key);
        strcpy(head->value, value);
        e[index] = head;
        return;
    }
    while (head)
    {
        if (!strcmp(key, head->key))
        {
            free(head->value);
            head->value = (char *)malloc(valuelen);
            strcpy(head->value, value);
            return;
        }
        if (head->next == nullptr)
        {
            break;
        }
        else
        {
            head = head->next;
        }
    }
    head->next = (st_ch_hashelement *)malloc(sizeof(st_ch_hashelement));
    head = head->next;
    head->key = (char *)malloc(len);
    head->value = (char *)malloc(valuelen);
    head->keylen = len;
    head->vlen = valuelen;
    head->next = nullptr;
    strcpy(head->value, value);
    strcpy(head->key, key);
    return;
}

int ch_hashmap_insert(st_ch_hashmap *map, const char *key, int keylen, const char *value, int valuelen)
{
    if (map->filled_len + 1 == map->tablelen)
    {
        ch_rehash(map);
    }

    int hash = ch_worst_hash(map->tablelen, key, keylen);
    ch_insert_hashelement(map->table, hash, key, keylen, value, valuelen);
    return 0;
}
