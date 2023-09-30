// len must be only of the string part. if the buffer includes '\0' then do not include it in length :)
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define INVALID_URL -1;

int decode_url(char *buffer, int start, int end, int len)
{
    int offset = 0;
    for (int i = start; i < end; i++)
    {
        if (buffer[i] == '%')
        {
            char *hex = (char *)malloc(3);
            char *endptr;
            memcpy(hex, buffer + i + 1, 2);
            hex[2] = '\0';
            char c = (char)strtol(hex, &endptr, 16);
            if (*endptr != '\0' && *endptr != '\n')
            {
                return INVALID_URL;
            }
            buffer[i - offset] = c;
            offset += 2;
            i += 2;
        }
        else
        {
            buffer[i - offset] = buffer[i];
        }
    }
    return offset;
}