#include "../src/utils/base64.cpp"

#include "../src/utils/sha1.cpp"
int main()
{
    char const string[] = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char result[21];
    SHA1(result, string, strlen(string));
    char hexresult[41];
    size_t offset;
    base64_encode((uint8_t *)result, strlen(result), hexresult);
    printf("%s\n", hexresult);
    printf("%d\n", strlen(result));
}