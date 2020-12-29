//
//  VCDIFF.c
//  Delta
//
//  Created by 邹子豪 on 12/28/20.
//

#include "VCDIFF.h"


D_RT window_packer(FILE *delta,stream *stm){
    return D_OK;
    
}


typedef struct _code{
    byte WIN_INDICATOR;
    size_t SOURCE_SEGMENT_LENGTH;
    uint64_t SOURCE_SEGMENT_POSITION;
    size_t LEN_CODE;
    size_t TARGET_LEN;
    byte DELTA_INDICATOR;
    size_t LEN_DATA;
    size_t LEN_INST;
    size_t LEN_ADDR;
    byte *DATA;
    byte *INST;
    byte *ADDR;
    
    
    struct _code_node *HEAD;
    struct _code_node *TAIL;
} code;
typedef struct _code_node{
    byte CODE;
    uint32_t SIZE1;
    uint32_t SIZE2;
    data_addr DATAADDR;
    uint32_t CODE_SIZE;
    struct _code_node *NEXT;
} code_node;

#if s_near<=0
#define s_near 4
#endif
#if s_same<=0
#define s_same 3
#endif
typedef struct _addr_cache{
    uint64_t near[s_near];
    int next_slot;
    uint64_t same[s_same*256];
} addr_cache;
D_RT cache_init(addr_cache *cc){
    int i;
    cc->next_slot=0;
    for(i=0;i<s_near;i++)cc->near[i]=0;
    for (i=0; i<s_same*256; i++)cc->same[i]=0;
    return D_OK;
}
D_RT cache_update(addr_cache *cc,uint64_t addr){
    cc->near[cc->next_slot]=addr;
    cc->next_slot=(cc->next_slot+1)%s_near;
    cc->same[addr%(s_same*256)]=addr;
    return D_OK;
}
#define VCD_SELF 0
#define VCD_HERE 1
uint64_t addr_encode(addr_cache* cc, uint64_t addr, uint64_t here, uint8_t* mode){
    int i;
    uint64_t d,bestd;
    uint8_t bestm;
    bestd=addr;
    bestm=VCD_SELF;
    if((d=here-addr)<bestd){
        bestd = d; bestm = VCD_HERE;
    }
    for(i = 0; i < s_near; ++i)if((d=addr-cc->near[i])>=0 && d<bestd){
        bestd=d;
        bestm=i+2;
    }
    if(cc->same[d=addr%(s_same*256)]==addr){
        bestd=d%256;
        bestm=s_near+2+d/256;
    }
    cache_update(cc, addr);
    *mode=bestm;
    return bestd;
}
uint32_t count_int_len(uint32_t integer){
    uint32_t len=0;
    while(integer){
        integer=integer>>7;
        len++;
    }
    return len;
}
uint64_t count_int64_len(uint64_t integer){
    uint32_t len=0;
    while(integer){
        integer=integer>>7;
        len++;
    }
    return len;
}
void count_int_len_test(void){
    uint32_t test[3]={127,16383,2097151};
    for(int x=0;x<3;x++){
        printf("number %d length is %d\n",test[x],count_int_len(test[x]));
    }
}
D_RT add_single_code(code *cod,byte instcode,uint32_t size1,data_addr dataaddr){
    code_node *new=(code_node *)malloc(sizeof(code_node));
    new->CODE=instcode;
    new->SIZE1=size1;
    new->SIZE2=0;
    new->DATAADDR=dataaddr;
    new->NEXT=NULL;
    new->CODE_SIZE=1;//指令code的大小
    if(DeltaDefaultCodeTable[2][instcode]==0){
        new->CODE_SIZE+=count_int_len(size1);//如果指令code中的code选项为0，则需要额外的size字节
    }
    if(6<=DeltaDefaultCodeTable[4][instcode] && DeltaDefaultCodeTable[4][instcode]<=8){
        new->CODE_SIZE++;//如果mode为VCD_SAME，其addr大小永远为1gebyte
    }else{
        if(DeltaDefaultCodeTable[0][instcode]==COPY){
            new->CODE_SIZE+=count_int64_len(dataaddr.addr);//如果为其他mode，其大小随着addr大小变化，我们需要手动计算；
        }else if(DeltaDefaultCodeTable[0][instcode]==ADD){
            new->CODE_SIZE+=size1;//如果指令为add，则需要加上数据大小
        }else{
            new->CODE_SIZE++;//如果指令为run，则需要加上1个byte
        }
    }
    
    if(cod->TAIL==NULL && cod->HEAD==NULL){
        cod->HEAD=new;
        cod->TAIL=new;
    }else{
        cod->TAIL->NEXT=new;
    }
    return D_OK;
}
D_RT code_instruction(instruction *inst,code *cod){
    data_addr local;
    uint64_t local_mask=inst->START_POSITION;
    uint8_t mode;
    byte instcode;
    instruction_node *curr_nd=inst->HEAD;
    addr_cache cache;
    cache_init(&cache);
    while(curr_nd!=NULL){
        if(curr_nd->INST_TYPE==COPY){//MARK: 未来此处可能需要判断small match
            local.addr=curr_nd->DATA_or_ADDR.addr-local_mask;
            local.addr=addr_encode(&cache, local.addr, curr_nd->POSITION+inst->LENGTH, &mode);
            instcode=19+mode*16+((curr_nd->SIZE<=18 && curr_nd->SIZE>=4)?curr_nd->SIZE-3:0);
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
        }else if(curr_nd->INST_TYPE==RUN){
            local.data=curr_nd->DATA_or_ADDR.data;
            instcode=0;
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
        }else if(curr_nd->INST_TYPE==ADD){
            local.data=curr_nd->DATA_or_ADDR.data;
            instcode=(curr_nd->SIZE>=1 && curr_nd->SIZE<=17)?curr_nd->SIZE:0;
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
        }
    }
    return D_OK;
}


