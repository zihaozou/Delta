//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//

#include "type_def.h"
#include "crc16speed.h"
#include "SOURCE.h"

//static source *Source;
int main(int argc, const char * argv[]) {
    //crc16speed_init();
    //uint16_t x=crc16speed(0, "abcdabcd", 8);
    source *Source=create_source();
    set_src_file(Source, "test");
    init_window(Source);
    //char x=get_char_at(Source, 1);
    //char buffer[8]={0};
    //get_n_char_at(Source, 509, buffer);
    global_source_hash(Source);
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
