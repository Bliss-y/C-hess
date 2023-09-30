#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ch_hash.cpp"
#define INVALID_REQUEST -1
// TURN THIS INTO MAP INSTEAD!
struct st_headers
{
    char *Accept;
    char *host;
    char *acceptEncoding;
    char *connection;
    char *contentType;
    char *contentLength;
    char *boundary;
};
struct st_html
{
    char *method;
    char *path;
    char *query;
    char *protocol;
    st_ch_hashmap *headers;
    char *body;
};
struct st_ht_stringbuffer
{
    int current;
    char *buffer;
    int len;
};

void free_stringbuffer(st_ht_stringbuffer *buffer);
void free_ht_struct(st_html *filler);
int chess_parse_html(const char *recb, int bufferlen, st_html *filler);
