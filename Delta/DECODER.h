//
//  DECODER.h
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#ifndef DECODER_h
#define DECODER_h

#include "type_def.h"
#include "STREAM.h"
#include "VCDIFF.h"


void test_read_integer(void);
void DECODER(const char *delta_name,const char *source_name,const char *updated_name);
#endif /* DECODER_h */
