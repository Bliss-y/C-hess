#include "../src/utils/ch_hash.cpp"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>

int main()
{
    printf("hello");
    while (true)
    {
        st_ch_hashmap *map = ch_create_hash_map();
        printf("First round hashing!\n");
        ch_hashmap_insert(map, "super", 6, "not super", 10);
        // ch_hashmap_insert(map, "what?", 6, "not what?", 10);
        // ch_hashmap_insert(map, "well", 5, "not well", 9);
        // ch_hashmap_insert(map, "hashed!", 8, "not hashed!", 12);
        // printf("cool: %s\n", ch_hash_get(map, "cool", 5)->value);
        // printf("super: %s\n", ch_hash_get(map, "super", 6)->value);
        // printf("what?: %s\n", ch_hash_get(map, "what?", 6)->value);
        // printf("well?: %s\n", ch_hash_get(map, "well", 5)->value);
        // printf("hashed!?: %s\n", ch_hash_get(map, "hashed!", 8)->value);
        // printf("Replacing hashes!\n");
        // ch_hashmap_insert(map, "cool", 5, "so cool", 8);
        // ch_hashmap_insert(map, "super", 6, "so super", 9);
        // ch_hashmap_insert(map, "what?", 6, "so what?", 9);
        // printf("cool: %s\n", ch_hash_get(map, "cool", 5)->value);
        // printf("super: %s\n", ch_hash_get(map, "super", 6)->value);
        // printf("what?: %s\n", ch_hash_get(map, "what?", 6)->value);

        // for (int i = 0; i < 100; i++)
        // {
        //     ch_hashmap_insert(map, "cool", 5, "not cool", 9);
        // }
        ch_destroy_hash(map);
    }
    printf("bye");
    return 0;
}
