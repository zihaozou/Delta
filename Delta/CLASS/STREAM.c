//
//  STREAM.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "STREAM.h"

stream *create_stream(void){
    stream *stm=(stream *)malloc(sizeof(stream));
    memset(stm, 0, sizeof(stream));
    return stm;
}
D_RT add_target(stream *stm, target *tgt){
    if(stm==NULL){
        printf("\nERROR: stream uninitialized\n");
        return D_ERROR;
    }else if(tgt==NULL){
        printf("\nERROR: target uninitialized\n");
    }
    stm->TARGET=tgt;
    return D_OK;
    
}
D_RT add_source(stream *stm, source *src){
    if(stm==NULL){
        printf("\nERROR: stream uninitialized\n");
        return D_ERROR;
    }else if(src==NULL){
        printf("\nERROR: source uninitialized\n");
    }
    stm->SOURCE=src;
    return D_OK;
}
D_RT init_encode(stream *stm){
    if(stm==NULL){
        printf("\nERROR: stream uninitialized\n");
        return D_ERROR;
    }else if(stm->SOURCE==NULL){
        printf("\nERROR: source uninitialized\n");
        return D_ERROR;
    }else if(stm->TARGET==NULL){
        printf("\nERROR: target uninitialized\n");
        return D_ERROR;
    }
    stm->INPUT_POSITION=0;
    stm->INPUT_REMAINING=stm->TARGET->TARGET_WINDOW->BUFFER_SIZE;
    return D_OK;
}
//TODO: 现在先只考虑large match的情况，等到完成了之后，需要添加RUN和small
//match，这三种情况会在匹配时全部进行一遍，然后由instuction里面的instruction_mediator调和三种情况的结果，选出最优
//解。在我们的情况中，我们无需考虑lazymatch的问题，因为在匹配阶段我们就已经找到了全局的最优解，可以直接跳过已匹配的部分，

#define MIN_RUN_LEN 4
//TODO: 还需搞清large match和small match之间的最优解原理
D_RT match(stream *stm){
    printf("\nSTRING MATCH: initial entry: %llu\n",stm->TARGET->TARGET_WINDOW->START_POSITION);
    char *tgt_buffer=stm->TARGET->TARGET_WINDOW->BUFFER;
    //uint64_t curr_posi=stm->INPUT_POSITION;
    uint16_t tgtcrc;
    source_hash *s;
    source_hash *hashtable=stm->SOURCE->SOURCE_HASH;
    instruction *inst=stm->TARGET->TARGET_WINDOW->INSTRUCTION;
    while(stm->INPUT_REMAINING>=MIN_RUN_LEN/*MARK: 这里未来可能会修改，需要判定匹配所需的最小值，这个最小值由RUN，small match和large match三者所影响，现在只考虑largematch的情况*/){
        
        
        /*MARK: RUN*/
        //TODO: 修改instructions mediator内的run逻辑
        if(stm->INPUT_REMAINING>=MIN_RUN_LEN){
            uint64_t curr=stm->INPUT_POSITION;
            uint64_t size=0;
            while(curr+1<=stm->TARGET->TARGET_WINDOW->BUFFER_SIZE-1){
                if(tgt_buffer[curr]==tgt_buffer[curr+1]){
                    size++;
                    curr++;
                }
                else break;
            }
            size++;
            if(size>=4){
                instruction_node *new=new_inst_node(NULL, RUN, stm->INPUT_POSITION, size, &tgt_buffer[stm->INPUT_POSITION], 0);
                add_instructions(inst, new, 1);
            }
        }
        
        /*MARK: LARGE_MATCH,这里重复检查一遍stm->INPUT_REMAINING>=CRC_LEN，为了以后上面的while判断条件可能会被修改*/
        if(stm->INPUT_REMAINING>=CRC_LEN){
            tgtcrc=crc16speed(0, &tgt_buffer[stm->INPUT_POSITION], CRC_LEN);
            HASH_FIND(hh, hashtable, &tgtcrc, 2, s);
            if(s!=NULL){
                instruction_node *inst_node=match_extend(stm->INPUT_POSITION,stm->TARGET->TARGET_WINDOW,stm->SOURCE,s);
                if(inst_node!=NULL)add_instructions(stm->TARGET->TARGET_WINDOW->INSTRUCTION, inst_node, 1);
                
            }
        }
        
        /*MARK: small match*/
        //通常window窗口都很小
        
        
        
        
        //更新inputposition
        if(inst->INSTRUCTION_COUNT!=0 && inst->TAIL->POSITION+
           inst->TAIL->SIZE-1>=stm->INPUT_POSITION){
            stm->INPUT_POSITION=inst->TAIL->POSITION+inst->TAIL->SIZE;
        }else stm->INPUT_POSITION++;
        stm->INPUT_REMAINING=stm->TARGET->TARGET_WINDOW->BUFFER_SIZE-stm->INPUT_POSITION;
        //ram cost
    }
    
    
    
    
    return D_OK;
}

instruction_node *match_extend(uint64_t curr_posi,target_window *win,source *src,source_hash *sh){
    source_position *p=sh->head;
    uint64_t try_src_start;
    uint64_t try_tgt_start;
    uint64_t try_size;
    uint64_t try_size_max;
    //uint64_t try_src_end;
    //uint64_t try_tgt_end;
    uint64_t max_length=0;
    uint64_t tgt_start_point=0;
    uint64_t src_start_point=0;
    while(p!=NULL){
        //try_src_end=p->position;
        try_src_start=p->position;
        //try_tgt_end=curr_posi;
        try_tgt_start=curr_posi;
        try_size=0;
        try_size_max=delta_min(src->SOURCE_FILE->FILE_SIZE-try_src_start-1, win->BUFFER_SIZE-try_tgt_start-1);
        
        //FORWARD EXTEND
        while(try_size<=try_size_max&&get_char_at(src, (uint32_t)(try_src_start+try_size))==win->BUFFER[try_tgt_start+try_size]){
            try_size++;
        }
        
        
        
        //BACKWARD EXTEND
        while(try_src_start>0 && try_tgt_start>0){
            if(get_char_at(src, (uint32_t)try_src_start-1)==win->BUFFER[try_tgt_start-1]){
                try_src_start--;
                try_tgt_start--;
                try_size++;}
            else break;
        }
        
        //比较
        if(try_size>=max_length){
            tgt_start_point=try_tgt_start;
            src_start_point=try_src_start;
            max_length=try_size;
        }
        p=p->next;
    }
    if(max_length>=CRC_LEN/*暂时设置为crc的长度*/){
        instruction_node *inst_node=new_inst_node(NULL, COPY, tgt_start_point,max_length, NULL, src_start_point);
        return inst_node;
    }
    
    else return NULL;
    //再次检查总长度，如果超过阀值则生成node，如果不超过则返回NULL
}


void stream_match_test(void){
    FILE *output=fopen("38kB-54kB/2_1_unbm_delta_file", "wb+");
    stream *Stream=create_stream();
    target *Target=create_target();
    source *Source=create_source();
    //instruction *Instruction=create_instruction();
    set_src_file(Source, "38kB-54kB/unbm_R2.bin");
    init_window(Source);
    global_source_hash(Source);
    set_tgt_file(Target, "38kB-54kB/unbm_R1.bin");
    add_target(Stream, Target);
    add_source(Stream, Source);
    header_packer(output, Stream);
    
    while(set_window(Target)==D_OK){
        init_encode(Stream);
        match(Stream);
        ADD_complement(Stream->TARGET->TARGET_WINDOW->INSTRUCTION, Stream->TARGET->TARGET_WINDOW);
        window_packer(output, Stream);
    }
    fclose(output);
    delete_inst_list(Target->TARGET_WINDOW->INSTRUCTION);
    clean_source(Source);
    clean_target(Target);
    //TODO: ADD_complement和vcd_packer。完成后添加RUN和small match
}
