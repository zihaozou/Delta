//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//

#include "type_def.h"



int main(int argc, const char * argv[]) {
    FILE *testfile;
    testfile=fopen("test", "r+b");
    fseek(testfile,0,SEEK_END);
    size_t size=ftell(testfile);
    rewind(testfile);
    int block_size=SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER;
    int block_count=size/block_size;
    
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
