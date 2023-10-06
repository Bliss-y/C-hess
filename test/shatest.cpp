#include "../src/utils/sha1.cpp"

int main()
{
    char const string[] = "abc";
    char const expect[] = "a9993e364706816aba3e25717850c26c9cd0d89d";
    char result[21];
    char hexresult[41];
    size_t offset;

    /* calculate hash */
    SHA1(result, string, strlen(string));
    for (offset = 0; offset < 20; offset++)
    {
        sprintf((hexresult + (2 * offset)), "%02x", result[offset] & 0xff);
    }

    printf("%s", hexresult);
}