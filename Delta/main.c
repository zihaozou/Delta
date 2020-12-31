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
    //stream_match_test();
    //count_int_len_test();
    //vcd_test();
    // insert code here...
    //test_read_integer();
    DECODER("72kB-80kB/1_2_unbm_delta_file", "72kB-80kB/unbm_R1.bin", "72kB-80kB/1_2_unbm_updated.bin");
    printf("Hello, World!\n");
    return 0;
}
