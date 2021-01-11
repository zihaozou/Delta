/*
MIT License

Copyright (c) 2021 ZihaoZou

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
