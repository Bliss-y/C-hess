#include "base64.h"
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
  number of characters = 3;
  padding = 3 - 3%3 = 3?
  totalbits = 8*6 = 54?;
  base64 length = 8/3 * 4
*/

void base64_encode(const uint8_t *input, size_t input_len, char *output)
{
    for (size_t i = 0, j = 0; i < input_len;)
    {
        uint32_t octet_a = i < input_len ? input[i++] : 0;
        uint32_t octet_b = i < input_len ? input[i++] : 0;
        uint32_t octet_c = i < input_len ? input[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        output[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        output[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        output[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        output[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    size_t padding = (input_len % 3);
    for (size_t i = 0; i < padding; i++)
    {
        output[((input_len + 1) * 4 / 3) - padding + i] = '=';
    }
    output[(input_len / 3 + (input_len % 3 > 0 ? 1 : 0)) * 4] = '\0';
}
