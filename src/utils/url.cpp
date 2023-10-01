#include "url.h"
// len must be only of the string part. if the buffer includes '\0' then do not include it in length :)
char url_decode_percent_char(st_ht_stringbuffer *buffer)
{
    char *hex = (char *)malloc(3);
    char *endptr;
    memcpy(hex, buffer + buffer->current + 1, 2);
    hex[2] = '\0';
    char c = (char)strtol(hex, &endptr, 16);
    if (*endptr != '\0' && *endptr != '\n')
    {
        return INVALID_URL;
    }
    return c;
}

int url_parse_params(st_ht_stringbuffer *buffer, st_ch_hashmap *store)
{
    while (buffer->current < buffer->len)
    {
        if (buffer->buffer[buffer->current] == ' ' || buffer->buffer[buffer->current] == '\r' || buffer->buffer[buffer->current] == '\n')
        {
            return 0;
        }
        int keylen = 0;
        int valuelen = 0;
        if (buffer->buffer[buffer->current] == '&')
        {
            buffer->current++;
        }
        int start = buffer->current;
        while (buffer->buffer[buffer->current] != '=' && buffer->current < buffer->len)
        {
            if (buffer->buffer[buffer->current] == ' ' || buffer->buffer[buffer->current] == '\r' || buffer->buffer[buffer->current] == '\n')
            {
                return INVALID_URL;
            }
            keylen++;
            buffer->current++;
        }
        if (buffer->buffer[buffer->current] == '=')
        {
            buffer->current++;
            int offset = 0;
            while (buffer->buffer[buffer->current] != '&' && buffer->current < buffer->len && buffer->buffer[buffer->current] != ' ' && buffer->buffer[buffer->current] != '\r' && buffer->buffer[buffer->current] != '\n')
            {
                if (buffer->buffer[buffer->current] == '%')
                {
                    buffer->buffer[buffer->current - offset] = url_decode_percent_char(buffer);
                    buffer->current += 3;
                }
                else
                {
                    buffer->buffer[buffer->current - offset] = buffer->buffer[buffer->current];
                    buffer->current++;
                }
                valuelen++;
            }
        }
        else
        {
            return INVALID_URL;
        }
        char *key = (char *)malloc(keylen + 1);
        char *value = (char *)malloc(valuelen + 1);
        memcpy(key, buffer->buffer + start, keylen);
        // key offset + 1 for '='
        memcpy(value, buffer->buffer + start + keylen + 1, valuelen);
        key[keylen] = '\0';
        value[valuelen] = '\0';
        ch_hashmap_insert(store, key, keylen + 1, value, valuelen + 1);
    }
    return 0;
}