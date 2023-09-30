
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

void free_stringbuffer(st_ht_stringbuffer *buffer)
{
    free(buffer->buffer);
}

st_ch_hashelement *ht_get_header(st_html *filler, const char *key, int keylen)
{
    return ch_hash_get(filler->headers, key, keylen);
}

int go_to_next_word(st_ht_stringbuffer *buffer)
{
    int n_headers = 0;
    for (int i = buffer->current; i < buffer->len; i++)
    {
        if (buffer->buffer[i] == ' ' || buffer->buffer[i] == '\n')
        {
            if (buffer->buffer[i] == '\n')
            {
                n_headers++;
            }
            buffer->current++;
        }
        else
        {
            break;
        }
    }
    return n_headers;
}

int ht_wordlen(st_ht_stringbuffer *buffer)
{
    int len = 0;
    for (int i = buffer->current; i < buffer->len; i++)
    {
        if (buffer->buffer[i] == ' ' || buffer->buffer[i] == '\n' || buffer->buffer[i] == '\r')
        {
            break;
        }
        else
        {
            len++;
        }
    }
    return len;
}

// TODO: Write a ignore whitespace function for stringbuffer
int ht_get_next_header(st_ht_stringbuffer *stringbuffer, st_html *filler)
{
    // necessary to remove any whitespace first
    if (go_to_next_word(stringbuffer) > 1)
    {
        return 1;
    }
    // length of the header
    int len = 0;
    for (int i = stringbuffer->current; i < stringbuffer->len; i++)
    {
        if (stringbuffer->buffer[i] == '\n' || stringbuffer->buffer[i] == '\r')
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
    for (int i = tmpcur; i <= tmpcur + len; i++)
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
    title.buffer = (char *)malloc(sizeof(char) * (title.len + 1));
    memcpy(title.buffer, stringbuffer->buffer + tmpcur, sizeof(char) * title.len);
    title.buffer[title.len] = '\0';
    const char *x = title.buffer;
    // get the value
    for (int i = stringbuffer->current; i <= stringbuffer->current - title.len + len; i++)
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
    for (int i = stringbuffer->current; i < tmpcur + len; i++)
    {
        if (stringbuffer->buffer[i] == ' ' || stringbuffer->buffer[i] == '\n' || stringbuffer->buffer[i] == '\r')
        {
            break;
        }
        value.len++;
    }
    value.buffer = (char *)malloc(sizeof(char) * (value.len + 1));
    memcpy(value.buffer, stringbuffer->buffer + stringbuffer->current, sizeof(char) * value.len);
    value.buffer[value.len] = '\0';
    const char *y = value.buffer;
    stringbuffer->current += value.len;
    ch_hashmap_insert(filler->headers, x, title.len + 1, y, value.len + 1);
    free_stringbuffer(&title);
    free_stringbuffer(&value);
    return 0;
}

void free_ht_struct(st_html *filler)
{
    free(filler->method);
    filler->method = nullptr;
    if (filler->path)
    {
        free(filler->path);
        filler->path = nullptr;
    }
    if (filler->query)
    {
        free(filler->query);
        filler->query = nullptr;
    }
    if (filler->protocol)
    {
        free(filler->protocol);
        filler->protocol = nullptr;
    }
    if (filler->headers)
    {
        ch_destroy_hash(filler->headers);
        filler->headers = nullptr;
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
    char *path = (char *)malloc(len + 1);
    path[len] = '\0';
    memcpy(path, recb + current, sizeof(char) * (len));
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
    char *protocol = (char *)malloc(len + 1);
    protocol[len] = '\0';
    memcpy(protocol, recb + current, len);
    filler->protocol = protocol;
    filler->headers = ch_create_hash_map();
    st_ht_stringbuffer sb_rec = {};
    sb_rec.buffer = (char *)recb; // cpp is so stupid holy shit.
    sb_rec.current = current + len;
    sb_rec.len = bufferlen;
    while (!ht_get_next_header(&sb_rec, filler))
    {
    }
    return 0;
}
