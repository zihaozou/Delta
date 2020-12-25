//
//  INSTRUCTION.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "INSTRUCTION.h"


/// create_instruction:
/// 创建一个新的instruction实例，其内部全部被归零
///
instruction *create_instruction(void){
    instruction *inst=(instruction *)malloc(sizeof(instruction));
    memset(inst, 0, sizeof(instruction));
    return inst;
    
}
/// add_instructions
/// 为已有的instruction list添加新的指令，可以为数个指令
/// @param inst 指令list实例
/// @param inst_node 新的指令
/// @param number 新指令的数量
D_RT add_instructions(instruction *inst, instruction_node *inst_node,int number){
    int x=0;
    while(inst_node!=NULL && x<number){
        instructions_mediator(inst,inst_node);//首先调和新COPY和旧指令之间的指令冲突
        inst_node->PREV=inst->TAIL;//让新指令的prev指针指向list的末尾，如果末尾为空的话，那就会指向NULL
        if(inst->INSTRUCTION_COUNT==0)inst->HEAD=inst_node;//如果指令list为空，则将指令头指向新指令
        else{//如果不为空，则将指令尾的元素的下一指令指针指向新指令
            inst->TAIL->NEXT=inst_node;
        }
        inst->TAIL=inst_node;//将指令list的指令尾指向新指令
        if(inst_node->INST_TYPE==COPY){//计算指令list所涉及所有指令在源文件中占用的片段长度
            if(!inst->FIRST_COPY){//如果这是这个指令list收到的第一个copy指令，则将片段的起始位置设置为这个指令所指向的
                //的源文件片段的开头，长度则为这个指令的长度
                inst->START_POSITION=inst_node->DATA_or_ADDR.addr;
                inst->LENGTH=inst_node->SIZE;
                inst->FIRST_COPY=1;
            }else{//如果这个指令不是第一个copy指令，则比较现有的起始位置和新指令的起始位置之间的最小值，
                //再比较跨度在最大值
                inst->START_POSITION=delta_min(inst->START_POSITION, inst_node->DATA_or_ADDR.addr);
                inst->LENGTH=delta_max(inst->LENGTH, inst_node->DATA_or_ADDR.addr+inst_node->SIZE-inst->START_POSITION);
            }
        }
        inst->INSTRUCTION_COUNT++;//增进指令计数
        inst_node=inst_node->NEXT;//进入下一新指令
        x++;//增进x
    }
    return D_OK;
}
D_RT delete_instruction(instruction *inst, instruction_node *inst_node){
    if(inst_node->PREV!=NULL)inst_node->PREV->NEXT=inst_node->NEXT;
    if(inst_node->NEXT!=NULL)inst_node->NEXT->PREV=inst_node->PREV;
    if(inst->HEAD==inst_node)inst->HEAD=inst_node->NEXT;
    if(inst->TAIL==inst_node)inst->TAIL=inst_node->PREV;
    free(inst_node);
    return D_OK;
}
instruction_node *new_inst_node(instruction_node *prev/*可选*/,inst_type type,
                                uint32_t posi,//此指令在源文件中的起始位置
                                uint32_t size,
                                char *data,uint32_t addr){
    instruction_node *ins_nd=(instruction_node *)malloc(sizeof(instruction_node));
    ins_nd->PREV=prev;
    if(prev!=NULL)prev->NEXT=ins_nd;
    ins_nd->NEXT=NULL;
    ins_nd->POSITION=posi;
    ins_nd->SIZE=size;
    ins_nd->INST_TYPE=type;
    if(type==COPY)ins_nd->DATA_or_ADDR.addr=addr;
    else ins_nd->DATA_or_ADDR.data=data;
    return ins_nd;
}
/// instructions_mediator：
/// 这个函数是为了调和指令冲突
/// TODO: 需要更多的说明
/// @param inst <#inst description#>
/// @param inst_node <#inst_node description#>
D_RT instructions_mediator(instruction *inst, instruction_node *inst_node){
    if(inst->INSTRUCTION_COUNT==0 || inst->TAIL==NULL)return D_EMPTY;
    if(inst_node->INST_TYPE!=COPY)return D_OK;
    instruction_node *temp=inst->TAIL;
    instruction_node *temp2;
    while(temp!=NULL){
        temp2=temp;
        temp=temp->PREV;
        uint32_t endpoint=temp2->POSITION+temp2->SIZE-1;
        if(inst_node->POSITION<=endpoint){
            //判断insttype，run或add直接覆盖，copy必须完全包含了原指令才能覆盖
            switch (temp2->INST_TYPE) {
                case ADD:
                case RUN://考虑直接删除还是部分覆盖
                    if(inst_node->POSITION<=temp2->POSITION){
                        delete_instruction(inst,temp2);//直接删除
                    }else{
                        temp2->SIZE=temp2->SIZE-(endpoint-inst_node->POSITION+1);
                    }
                    break;
                case COPY:
                    if(inst_node->POSITION<=temp2->POSITION){//完全覆盖了原COPY指令，删除原COPY
                        delete_instruction(inst, temp2);
                    }else{//没有完全覆盖，修改新COPY指令的target起始地址和size和addr
                        uint32_t cover=endpoint-inst_node->POSITION+1;
                        inst_node->SIZE-=cover;
                        inst_node->POSITION+=cover;
                        inst_node->DATA_or_ADDR.addr+=cover;
                    }
                    break;
                default:
                    break;
            }
        }else{
            break;
        }
    }
    return D_OK;
}
