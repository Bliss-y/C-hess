#include "../src/utils/parsehttp.cpp"
#include "string.h"

const char *req1 = "GET /?a=b HTTP/1.1\nUser-Agent: PostmanRuntime/7.29.2\nAccept: */*\nPostman-Token: f20bdbb7-668d-461a-b679-bd91fefac556\nHost: localhost:3000\nAccept-Encoding: gzip, deflate, br\nConnection: keep-alive\nContent-Type : multipart / form - data;\r\n\r\nboundary = -- -- -- -- -- -- -- -- -- -- -- -- --175365135323593656885807 Content - Length : 359";
const char *req2 = "POST /?a=b HTTP/1.1\nUser-Agent: PostmanRuntime/7.29.2\nAccept: */*\nPostman-Token: f20bdbb7-668d-461a-b679-bd91fefac556\nHost: localhost:3000\nAccept-Encoding: gzip, deflate, br\nConnection: keep-alive\nContent-Type : multipart / form - data;\r\nboundary = -- -- -- -- -- -- -- -- -- -- -- -- --175365135323593656885807 Content - Length : 359";
const char *fail1 = "GET";
const char *fail2 = "G";
const char *fail3 = "";
const char *fail4 = "asdfasfd asdf asdf asdfd";

int main()
{
    while (true)
    {
        st_html filler = {};
        int x = chess_parse_html(req1, 100, &filler);

        if (x == 0 && !strcmp(filler.method, "GET ") && !strcmp(filler.path, "/?a=b") && !strcmp(filler.protocol, "HTTP/1.1"))
        {
            printf("test0 pass\n");
        }
        else
        {

            printf("test0 failed\n");
            printf("x value is: %d\n", x);
            printf("method value is: %s", filler.method);
            printf("path value is: %s", filler.path);
            printf("protocol value is: %s", filler.protocol);
        }
        free_ht_struct(&filler);
        x = chess_parse_html(fail1, 3, &filler);
        if (x == INVALID_REQUEST)
        {
            printf("test1 pass\n");
        }
        else
        {
            printf("test1 failed\n");
        }
        free_ht_struct(&filler);
    }
}
