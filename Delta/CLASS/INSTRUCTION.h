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
//  INSTRUCTION.h
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#ifndef INSTRUCTION_h
#define INSTRUCTION_h

#include "type_def.h"

instruction *create_instruction(void);
D_RT add_instructions(instruction *inst, instruction_node *inst_node,int number);
instruction_node *new_inst_node(instruction_node *prev/*可选*/,inst_type type,
                                uint64_t posi,//此指令在源文件中的起始位置
                                uint64_t size,
                                char *data,uint64_t addr);
D_RT instructions_mediator(instruction *inst, instruction_node *inst_node);
D_RT delete_instruction_node(instruction *inst, instruction_node *inst_node);
D_RT delete_inst_list(instruction *inst);//TODO: 实现删除操作
uint32_t ADD_complement(instruction *inst,target_window *win);
#endif /* INSTRUCTION_h */
