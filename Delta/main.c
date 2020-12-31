//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//
//#include "SOURCE.h"
//#include "INSTRUCTION.h"
//#include "TARGET.h"
#include "STREAM.h"
#include "VCDIFF.h"
#include "DECODER.h"
//static source *Source;
int main(int argc, const char * argv[]) {
    stream_match_test();
    //count_int_len_test();
    //vcd_test();
    // insert code here...
    //test_read_integer();
    DECODER("38kB-54kB/2_1_unbm_delta_file", "38kB-54kB/unbm_R2.bin", "38kB-54kB/2_1_unbm_updated.bin");
    printf("Hello, World!\n");
    return 0;
}
