#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "../src/utils/url.cpp"
#include "../src/utils/ch_hash.cpp"
#include "../src/utils/parsehttp.cpp"

int main()
{
    while (true)
    {
        char *request = "POST /api/endpoint HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 23\r\n\r\nkey1=value1&key2=value2";
        char *r = (char *)malloc(142);
        memcpy(r, request, 142);
        st_html filler = {};
        chess_parse_html(r, 142, &filler);
        free_ht_struct(&filler);
    }
    return 0;
}
