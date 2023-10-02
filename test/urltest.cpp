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
        const char *request = "POST /api/endpoint HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 23\r\n\r\nkey1=value1&key2=value2";
        char *r = (char *)malloc(142);
        memcpy(r, request, 142);
        st_html filler = {};
        chess_parse_html(r, 142, &filler);
        free(r);
        free_ht_struct(&filler);
        const char *p_request = "POST /api/endpoint HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 32\r\n\r\nkey1=value%201&key2=value%202";
        r = (char *)malloc(148);
        memcpy(r, p_request, 148);
        chess_parse_html(r, 148, &filler);
        printf("%s", ht_get_body(&filler, "key1", 5)->value);
        free(r);
    }
    return 0;
}
