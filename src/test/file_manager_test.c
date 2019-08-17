#include "file_manager.h"
#include <stdlib.h>

void run_file_manager_tests(){
    //todo add tests
}

int 
test_read_write_under_4098(){
    FILE* fp = create_file("/home/duler/Desktop/Test.txt", 3245);
    size_t *start = 0;
    BYTE* buffer = malloc(3245 + 1);
    read_file(fp, buffer, start, 3245);
}



FILE* 
create_file(const char* location, long size){
    FILE* fp;
    fp = fopen(location, "w");
    for(int i = 0; i < size; i++){
        fputc('a', fp);
    }
    return fp;
}

unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}