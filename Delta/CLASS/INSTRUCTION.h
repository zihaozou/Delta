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
                                uint32_t posi,//此指令在源文件中的起始位置
                                uint32_t size,
                                char *data,uint32_t addr);
D_RT instructions_mediator(instruction *inst, instruction_node *inst_node);
D_RT delete_instruction(instruction *inst, instruction_node *inst_node);
#endif /* INSTRUCTION_h */
