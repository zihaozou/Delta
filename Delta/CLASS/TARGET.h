//
//  TARGET.h
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#ifndef TARGET_h
#define TARGET_h
#include "type_def.h"
#include "INSTRUCTION.h"
target *create_target(void);
D_RT set_tgt_file(target *tgt,const char *filename);
D_RT set_window(target *tgt);
D_RT clean_target(target *tgt);

#endif /* TARGET_h */
