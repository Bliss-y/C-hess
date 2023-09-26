
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ch_hash.cpp";
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
    st_ch_hashmap headers;
    char *body;
};
struct st_ht_stringbuffer
{
    int current;
    char *buffer;
    int len;
};
void free_stringbuffer(st_ht_stringbuffer *buffer)
{
    free(buffer->buffer);
}

// TODO: Write a ignore whitespace function for stringbuffer
int ht_get_next_header(st_ht_stringbuffer *stringbuffer, st_html *filler)
{
    // necessary to remove any whitespace first
    for (int i = stringbuffer->current; i < stringbuffer->len; i++)
    {
        if (stringbuffer->buffer[i] == ' ' || stringbuffer->buffer[i] == '\n')
        {
            stringbuffer->current++;
        }
        else
        {
            break;
        }
    }
    // length of the header
    int len;
    for (int i = stringbuffer->current; i < stringbuffer->len; i++)
    {
        if (stringbuffer->buffer[i] == '\n')
        {
            break;
        }
        len++;
    }
    if (len == 0)
    {
        return INVALID_REQUEST;
    }

    // get the title
    st_ht_stringbuffer title = {};
    title.len = 0;
    int tmpcur = stringbuffer->current;
    for (int i = tmpcur; i < len; i++)
    {
        if (stringbuffer->buffer[i] == ' ' || stringbuffer->buffer[i] == ':')
        {
            break;
        }
        title.len++;
    }
    stringbuffer->current += title.len;
    if (title.len == 0)
    {
        return INVALID_REQUEST;
    }
    // get the value
    for (int i = title.len + tmpcur; i < len; i++)
    {
        if (stringbuffer->buffer[i] == ' ' || stringbuffer->buffer[i] == ':')
        {
            stringbuffer->current++;
        }
        else
        {
            break;
        }
    }
    st_ht_stringbuffer value = {};
    value.len = 0;
    value.current = 0;
    for (int i = stringbuffer->current; i < len; i++)
    {
        if (stringbuffer->buffer[i] == ' ')
        {
            break;
        }
        value.len++;
    }
}

void free_ht_struct(st_html *filler)
{
    free(filler->method);
    if (filler->path)
    {
        free(filler->path);
    }
    if (filler->query)
    {
        free(filler->query);
    }
}

// its going to parse the html message only extract whats needed discard the rest!
int chess_parse_html(const char *recb, int bufferlen, st_html *filler)
{
    int current = 0;
    if (bufferlen < 15)
    {
        return INVALID_REQUEST;
    }
    char *method = (char *)malloc(5);
    method[4] = '\0';
    // read the first line
    memcpy(method, recb, sizeof(char) * 4);
    if (strcmp(method, "GET ") && strcmp(method, "POST"))
    {
        return INVALID_REQUEST;
    }
    filler->method = method;
    current = 4;
    int len = 0;
    // ignoring all the whitespaces
    for (int i = 4; i < bufferlen; i++)
    {
        if (recb[i] == ' ')
        {
            current++;
        }
        else
        {
            break;
        }
    }

    for (int i = current; i < bufferlen; i++)
    {
        if (recb[i] == ' ')
        {
            break;
        }

        len++;
    }
    if (len == 0)
    {
        return INVALID_REQUEST;
    }
    char *path = (char *)malloc(len);
    path[len] = '\0';
    memcpy(path, recb + current, sizeof(char) * len);
    filler->path = path;
    current += len;
    len = 0;
    for (int i = current; i < bufferlen; i++)
    {
        if (recb[i] == ' ')
        {
            current++;
        }
        else
        {
            break;
        }
    }

    for (int i = current; i < bufferlen; i++)
    {
        if (recb[i] == '\n')
        {
            break;
        }

        len++;
    }
    if (len == 0)
    {
        return INVALID_REQUEST;
    }
    char *protocol = (char *)malloc(len);
    protocol[len] = '\0';
    memcpy(protocol, recb + current, len);
    filler->protocol = protocol;
    return 0;
}
