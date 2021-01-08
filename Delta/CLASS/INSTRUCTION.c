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
D_RT delete_inst_list(instruction *inst){
    instruction_node *curr;
    instruction_node *next;
    curr=inst->HEAD;
        
    while(curr!=NULL){
        next=curr->NEXT;
        free(curr);
        curr=next;
    }
    free(inst);
    return D_OK;
}
/// add_instructions
/// 为已有的instruction list添加新的指令，可以为数个指令
/// @param inst 指令list实例
/// @param inst_node 新的指令
/// @param number 新指令的数量
D_RT add_instructions(instruction *inst, instruction_node *inst_node,int number){
    int x=0;
    while(inst_node!=NULL && x<number){
        if(instructions_mediator(inst,inst_node)==D_DISCARD){
            instruction_node *temp=inst_node;
            inst_node=inst_node->NEXT;//进入下一新指令
            x++;
            free(temp);
            continue;
        }//首先调和新COPY和旧指令之间的指令冲突
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
                uint64_t end_point=inst->START_POSITION+inst->LENGTH;
                
                inst->START_POSITION=delta_min(inst->START_POSITION, inst_node->DATA_or_ADDR.addr);
                inst->LENGTH=delta_max(end_point-inst->START_POSITION, inst_node->DATA_or_ADDR.addr+inst_node->SIZE-inst->START_POSITION);
            }
        }
        inst->INSTRUCTION_COUNT++;//增进指令计数
        if(inst_node->INST_TYPE==COPY){
            printf("\n*******************\nNEW INSTRUCTION: a new instruction has added\nTYPE:COPY\nPOSITION:%llu\nSIZE:%llu\nADDR:%llu\n*******************\n",inst_node->POSITION,inst_node->SIZE,
                   inst_node->DATA_or_ADDR.addr);
        }else if(inst_node->INST_TYPE==SCOPY){
            printf("\n*******************\nNEW INSTRUCTION: a new instruction has added\nTYPE:SCOPY\nPOSITION:%llu\nSIZE:%llu\nADDR:%llu\n*******************\n",inst_node->POSITION,inst_node->SIZE,
                   inst_node->DATA_or_ADDR.addr);
        }else{
            printf("\n*******************\nNEW INSTRUCTION: a new instruction has added\nTYPE:ADD or RUN\n*******************\n");
        }
        
        inst_node=inst_node->NEXT;//进入下一新指令
        x++;//增进x
    }
    return D_OK;
}
/// delete_instruction：
/// 删除instruction node
/// @param inst <#inst description#>
/// @param inst_node <#inst_node description#>
D_RT delete_instruction_node(instruction *inst, instruction_node *inst_node){
    if(inst_node->PREV!=NULL)inst_node->PREV->NEXT=inst_node->NEXT;
    if(inst_node->NEXT!=NULL)inst_node->NEXT->PREV=inst_node->PREV;
    if(inst->HEAD==inst_node)inst->HEAD=inst_node->NEXT;
    if(inst->TAIL==inst_node)inst->TAIL=inst_node->PREV;
    inst->INSTRUCTION_COUNT--;
    //TODO: 重新计算跨度和起始位置
//    if(inst_node->POSITION==inst->START_POSITION || inst->START_POSITION+inst->LENGTH==inst_node->POSITION+inst_node->SIZE){
//        instruction_node *curr=inst->HEAD;
//        uint64_t start=curr->POSITION;
//        size_t size=curr->SIZE;
//        while(curr!=NULL){
//            start=delta_min(start, curr->POSITION);
//            size=delta_max(size, curr->POSITION+curr->SIZE-start);
//            curr=curr->NEXT;
//        }
//        inst->START_POSITION=start;
//        inst->LENGTH=size;
//    }
    free(inst_node);
    return D_OK;
}
instruction_node *new_inst_node(instruction_node *prev/*可选*/,inst_type type,
                                uint64_t posi,//此指令在目标文件中的起始位置
                                uint64_t size,
                                char *data,uint64_t addr){
    instruction_node *ins_nd=(instruction_node *)malloc(sizeof(instruction_node));
    ins_nd->PREV=prev;
    if(prev!=NULL)prev->NEXT=ins_nd;
    ins_nd->NEXT=NULL;
    ins_nd->POSITION=posi;
    ins_nd->SIZE=size;
    ins_nd->INST_TYPE=type;
    if(type==COPY || type==SCOPY)ins_nd->DATA_or_ADDR.addr=addr;
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
    if(inst_node->INST_TYPE==ADD)return D_OK;
    instruction_node *temp=inst->TAIL;
    instruction_node *temp2;
    while(temp!=NULL){
        temp2=temp;
        temp=temp->PREV;
        uint64_t endpoint=temp2->POSITION+temp2->SIZE-1;
        if(inst_node->POSITION<=endpoint){
            //判断insttype，run或add直接覆盖，copy必须完全包含了原指令才能覆盖
            switch (temp2->INST_TYPE) {
                case ADD:
                case RUN://考虑直接删除还是部分覆盖
                    if(inst_node->POSITION<=temp2->POSITION){
                        delete_instruction_node(inst,temp2);//直接删除
                    }else{
                        temp2->SIZE=temp2->SIZE-(endpoint-inst_node->POSITION+1);
                    }
                    break;
                case SCOPY:
                case COPY:
                    if(inst_node->POSITION<=temp2->POSITION){//完全覆盖了原COPY指令，删除原COPY
                        delete_instruction_node(inst, temp2);
                    }else{//没有完全覆盖，修改新COPY指令的target起始地址和size和addr
                        uint64_t cover=endpoint-inst_node->POSITION+1;
                        if(cover>inst_node->SIZE)
                            return D_DISCARD;
                        //这里有bug，size减完直接变负数了
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






uint32_t ADD_complement(instruction *inst,target_window *win){
    uint64_t next_start=0;
    instruction_node *curr=inst->HEAD;
    uint32_t add_size=0;
    while(curr!=NULL){
        if(next_start<curr->POSITION){
            instruction_node *new_add=new_inst_node(NULL, ADD, next_start, curr->POSITION-next_start, &win->BUFFER[next_start], 0);
            if(curr==inst->HEAD){
                inst->HEAD=new_add;
            }else{
                curr->PREV->NEXT=new_add;
                new_add->PREV=curr->PREV;
            }
            curr->PREV=new_add;
            new_add->NEXT=curr;
            inst->INSTRUCTION_COUNT++;
            
        }
        next_start=curr->POSITION+curr->SIZE;
        curr=curr->NEXT;
    }
    //TODO: 添加ADD最后一个时有BUG
    if(inst->TAIL!=NULL){
        uint64_t final=inst->TAIL->POSITION+inst->TAIL->SIZE;
        if(win->BUFFER_SIZE>final){
            instruction_node *new_add=new_inst_node(NULL, ADD, final, win->BUFFER_SIZE-final, &win->BUFFER[final], 0);
            inst->TAIL->NEXT=new_add;
            new_add->PREV=inst->TAIL;
            inst->TAIL=new_add;
            inst->INSTRUCTION_COUNT++;
        }
        
    }else{
        instruction_node *new_add=new_inst_node(NULL, ADD, 0, win->BUFFER_SIZE, win->BUFFER, 0);
        inst->TAIL=new_add;
        inst->HEAD=new_add;
        inst->INSTRUCTION_COUNT++;
    }
    
    return D_OK;
}
