#include "../src/utils/base64.cpp"

int main()
{
    char const string[] = "abc";
    char result[21];
    char hexresult[41];
    size_t offset;
    base64_encode((uint8_t *)string, strlen(string), hexresult);
    printf("%s", hexresult);
}