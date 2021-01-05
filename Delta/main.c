//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//
//#include "SOURCE.h"
//#include "INSTRUCTION.h"
//#include "TARGET.h"
//#include "STREAM.h"
//#include "VCDIFF.h"
#include "DECODER.h"
#include "ENCODER.h"
//static source *Source;
int main(int argc, const char * argv[]) {
    //ENCODER("test/old.bin","test/new.bin","test/delta_file");
    DECODER("4k_4k_unbm/4k_4k_unbm_delta_file", "4k_4k_unbm/4k_4k_unbm_R1.bin", "4k_4k_unbm/4k_4k_unbm_updated.bin");
    printf("\nDONE\n");
    return 0;
}
