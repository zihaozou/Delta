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
#include "SOURCE.h"
#include "INSTRUCTION.h"
#include "TARGET.h"
#include "VCDIFF.h"
stream *create_stream(void);
D_RT add_target(stream *stm, target *tgt);
D_RT add_source(stream *stm, source *src);
D_RT init_encode(stream *stm);//这个函数会检查encode之前所有的准备程序是否完成，并将input_position
                            //和input_remaining设置好
D_RT match(stream *stm);//这个函数会从input_position开始
//循环查找一个与源文件哈希表匹配的目标子字符串
instruction_node *match_extend(uint8_t issource,uint64_t curr_posi,target_window *win,source *src,source_hash *sh);
D_RT rearrange_source_file(stream *stm, uint32_t add_size);
//void stream_match_test(void);
#endif /* STREAM_h */
