//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//
#include "SOURCE.h"
#include "INSTRUCTION.h"
#include "TARGET.h"
//static source *Source;
int main(int argc, const char * argv[]) {
    //crc16speed_init();
    //uint16_t x=crc16speed(0, "abcdabcd", 8);
    //source *Source=create_source();
    //set_src_file(Source, "test");
    //init_window(Source);
    //char x=get_char_at(Source, 1);
    //char buffer[8]={0};
    //get_n_char_at(Source, 509, buffer);
    //global_source_hash(Source);
//    instruction *Inst=create_instruction();
//    instruction_node *node1=new_inst_node(NULL, COPY, 5, 10, NULL, 5);
//    instruction_node *node2=new_inst_node(node1, COPY, 13, 10, NULL, 100);
//    add_instructions(Inst, node1, 2);
    target *Target=create_target();
    set_tgt_file(Target, "test");
    set_window(Target);
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
