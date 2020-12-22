//
//  STREAM.h
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#ifndef STREAM_h
#define STREAM_h

#include <stdio.h>
#include "type_def.h"


stream *create_stream(void);
D_RT add_target(stream *STREAM, target *TARGET);
#endif /* STREAM_h */
