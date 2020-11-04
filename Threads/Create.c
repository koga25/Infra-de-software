#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main()
{
    char* string = "abcdabbasdfqabvceoabpkabab ab";
    FILE* file;
    
    file = fopen("file.txt", "wb");
    fprintf(file, "%s", string);
    /* code */
    return 0;
}
