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
//  VCDIFF.c
//  Delta
//
//  Created by 邹子豪 on 12/28/20.
//

#include "VCDIFF.h"


static uint64_t addr_encode(addr_cache* cc, uint64_t addr, uint64_t here, uint8_t* mode);
//static uint32_t count_int_len(uint32_t integer);

static D_RT add_single_code(code *cod,byte instcode,uint32_t size1,data_addr dataaddr);
static D_RT code_instruction(instruction *inst,code *cod);
static code *create_code(stream *stm);
static D_RT _write_integer(FILE * file,uint64_t integer,byte cnt);
static D_RT write_byte(FILE * file,byte Byte);
static D_RT write_bytes(FILE * file,char *buffer,int size);
static void content_writer(FILE *file,code *cod);
static void win_header_writer(FILE *file,code *cod);


D_RT header_packer(FILE *delta,stream *stm){
	byte blank_space[16]={0};
    byte temp=0x00;
    rewind(delta);
	write_bytes(delta,&blank_space[0],16);
	write_byte(delta, 0x00);//delta文件md5预留空间
	write_byte(delta, 0x00);//delta文件大小预留空间
	source_md5(stm->SOURCE,blank_space);
	write_bytes(delta,&blank_space[0],16);
	write_integer(delta, stm->SOURCE->SOURCE_FILE->FILE_SIZE); 	
    write_byte(delta, 0xd6);//V
    write_byte(delta, 0xc3);//C
    write_byte(delta, 0xc4);//D
    write_byte(delta, SOFTWARE_VERSION);
    //if(stm->DECOMPRESS)temp=temp | 0x01;
    //if(stm->CODETABLE)temp=temp | 0x02;
    write_integer(delta, stm->TARGET->TARGET_FILE->FILE_SIZE); 
    if(stm->ENCODE_MODE==1){
        temp|=0x04;
    }else if(stm->ENCODE_MODE==2){
        temp|=0x08;
    }else{
        temp|=0x10;
    }
    write_byte(delta, temp);//HDR_INDICATOR
    
    //MARK: 后期在此添加二次压缩和自定义指令表内容
    return D_OK;
}
void mode3_add_size_packer(FILE *delta,stream *stm,uint32_t add_size){
    write_integer(delta, add_size);
    return;
}
D_RT window_packer(FILE *delta,stream *stm){
    code *cod=create_code(stm);
    code_instruction(stm->TARGET->TARGET_WINDOW->INSTRUCTION, cod);
    win_header_writer(delta,cod);
    content_writer(delta,cod);
    clean_code(cod);
    return D_OK;
}


D_RT clean_code(code *cod){
    code_node *curr=cod->HEAD;
    code_node *next;
    while(curr!=NULL){
        next=curr->NEXT;
        free(curr);
        curr=next;
    }
    free(cod);
    return D_OK;
}

D_RT delta_md5(FILE *delta){
    MD5_CTX md5_checker;
    unsigned char check_buff[2048];
    unsigned char md5[16];
    uint32_t calculated=0;
    fseek(delta,0,SEEK_END);
    size_t size=ftell(delta)-18;
    if((size>>16)>0){
        printf("\nERROR: delta file size exceeds limit\n");
        return D_ERROR;
    }
    fseek(delta,18,SEEK_SET);
    MD5Init(&md5_checker);
    while(calculated<size){
        uint32_t copy_size=delta_min((uint32_t)size-calculated,2048);
        fread((void *)check_buff, 1, copy_size, delta);
        MD5Update(&md5_checker,check_buff,copy_size);
        calculated+=copy_size;
    }
    MD5Final(md5,&md5_checker);
    fseek(delta,0,SEEK_SET);
    write_bytes(delta,(char *)&md5[0],16);
    write_byte(delta,(size&0xff00)>>8);
    write_byte(delta,size&0xff);
    return D_OK;
}


/*内部函数*/

static D_RT add_single_code(code *cod,byte instcode,uint32_t size1,data_addr dataaddr){
    code_node *new=(code_node *)malloc(sizeof(code_node));
    new->CODE=instcode;
    new->SIZE1=size1;
    new->SIZE2=0;
    new->DATAADDR=dataaddr;
    new->NEXT=NULL;
    //new->DATA_ADDR_SIZE=0;
    if(DeltaDefaultCodeTable[2][instcode]==0){
        cod->LEN_INST+=count_int64_len(size1);
        //如果指令code中的code选项为0，则需要额外的size字节
    }
    cod->LEN_INST++;
    if(DeltaDefaultCodeTable[0][instcode]==COPY){
        if(6<=DeltaDefaultCodeTable[4][instcode] && DeltaDefaultCodeTable[4][instcode]<=8){
            //如果mode为VCD_SAME，其addr大小永远为1gebyte
            cod->LEN_ADDR+=1;
        }else{
            //如果为其他mode，其大小随着addr大小变化，我们需要手动计算；
            cod->LEN_ADDR+=count_int64_len(dataaddr.addr);
        }
    }else if(DeltaDefaultCodeTable[0][instcode]==ADD){
        //如果指令为add，则需要加上数据大小
        cod->LEN_DATA+=size1;
    }else{
        //如果指令为run，则需要加上1个byte
        cod->LEN_DATA+=1;
    }
    
    if(cod->TAIL==NULL && cod->HEAD==NULL){
        cod->HEAD=new;
        cod->TAIL=new;
        new->PREV=NULL;
    }else{
        cod->TAIL->NEXT=new;
        new->PREV=cod->TAIL;
        cod->TAIL=new;
    }
    return D_OK;
}
#define ENUM_NAME(x) case x: return(#x)
const char *print_type(inst_type insttype){
    switch (insttype) {
            ENUM_NAME(COPY);
            ENUM_NAME(ADD);
            ENUM_NAME(RUN);
            ENUM_NAME(NOOP);
            ENUM_NAME(SCOPY);
    }
    return NULL;
}
#define PRINT_DELTA(type,code) printf("\n*******************\nDELTA CODE:\nTYPE: %s\nCODE: %d\n*******************\n",print_type(type),code)
static D_RT code_instruction(instruction *inst,code *cod){
    data_addr local;
    uint64_t local_mask=inst->START_POSITION;
    uint8_t mode;
    //uint32_t size;
    byte instcode;
    instruction_node *curr_nd=inst->HEAD;
    addr_cache cache;
    cache_init(&cache);
    while(curr_nd!=NULL){//MARK: 未来此处可能需要判断small match
        if(curr_nd->INST_TYPE==COPY){
            local.addr=curr_nd->DATA_or_ADDR.addr-local_mask;
            local.addr=addr_encode(&cache, local.addr, curr_nd->POSITION+inst->LENGTH, &mode);
            instcode=19+mode*16+((curr_nd->SIZE<=18 && curr_nd->SIZE>=4)?curr_nd->SIZE-3:0);
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
            PRINT_DELTA(COPY,instcode);
        }else if(curr_nd->INST_TYPE==RUN){
            local.data=curr_nd->DATA_or_ADDR.data;
            instcode=0;
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
            PRINT_DELTA(RUN,instcode);
        }else if(curr_nd->INST_TYPE==ADD){
            local.data=curr_nd->DATA_or_ADDR.data;
            instcode=(curr_nd->SIZE>=1 && curr_nd->SIZE<=17)?curr_nd->SIZE+1:1;
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
            PRINT_DELTA(ADD,instcode);
        }else if (curr_nd->INST_TYPE==SCOPY){
            local.addr=curr_nd->DATA_or_ADDR.addr+inst->LENGTH;
            local.addr=addr_encode(&cache, local.addr, curr_nd->POSITION+inst->LENGTH, &mode);
            instcode=19+mode*16+((curr_nd->SIZE<=18 && curr_nd->SIZE>=4)?curr_nd->SIZE-3:0);
            add_single_code(cod, instcode, (uint32_t)curr_nd->SIZE, local);
            PRINT_DELTA(SCOPY,instcode);
        }
        
        curr_nd=curr_nd->NEXT;
    }
    
    return D_OK;
}


static code *create_code(stream *stm){
    code *new=(code *)malloc(sizeof(code));
    memset(new, 0, sizeof(code));
    new->WIN_INDICATOR=0x01;
    new->TARGET_LEN=stm->TARGET->TARGET_WINDOW->BUFFER_SIZE;
    new->SOURCE_SEGMENT_POSITION=stm->TARGET->TARGET_WINDOW->INSTRUCTION->START_POSITION;
    new->SOURCE_SEGMENT_LENGTH=stm->TARGET->TARGET_WINDOW->INSTRUCTION->LENGTH;
    return new;
}





static D_RT _write_integer(FILE * file,uint64_t integer,byte cnt){
    if(integer || cnt==0){
        byte temp=0x7f &integer;
        if(cnt)temp=temp | 0x80;
        integer=integer>>7;
        _write_integer(file, integer,++cnt);
        fwrite(&temp, sizeof(byte), 1, file);
    }
    return D_OK;
}

static D_RT write_byte(FILE * file,byte Byte){
    fwrite(&Byte, sizeof(byte), 1, file);
    return D_OK;
}

static D_RT write_bytes(FILE * file,char *buffer,int size){
    fwrite(buffer, size, 1, file);
    return D_OK;
}



static void content_writer(FILE *file,code *cod){//先只考虑单指令
    code_node *curr=cod->HEAD;
    while(curr){
        if(DeltaDefaultCodeTable[0][curr->CODE]==ADD)
        write_bytes(file,curr->DATAADDR.data,curr->SIZE1);
        else if(DeltaDefaultCodeTable[0][curr->CODE]==RUN)
            write_byte(file, curr->DATAADDR.data[0]);
        curr=curr->NEXT;
    }
    curr=cod->HEAD;
    while(curr){
        write_byte(file,curr->CODE);
        if(!DeltaDefaultCodeTable[2][curr->CODE])write_integer(file,curr->SIZE1);
        //if(curr->SIZE2)write_byte(file,curr->SIZE2);
        curr=curr->NEXT;
    }
    curr=cod->HEAD;
    while(curr){
        if(DeltaDefaultCodeTable[0][curr->CODE]==COPY){
            if(DeltaDefaultCodeTable[4][curr->CODE]<6){
                write_integer(file, curr->DATAADDR.addr);
            }else{
                write_byte(file, (byte)curr->DATAADDR.addr);
            }
        }
        curr=curr->NEXT;
    }
    
}

static void win_header_writer(FILE *file,code *cod){
    write_byte(file, cod->WIN_INDICATOR);
    write_integer(file, cod->SOURCE_SEGMENT_LENGTH);
    write_integer(file, cod->SOURCE_SEGMENT_POSITION);
    cod->LEN_CODE=count_int64_len(cod->TARGET_LEN)+count_int64_len(cod->DELTA_INDICATOR)+count_int64_len(cod->LEN_INST)+count_int64_len(cod->LEN_DATA)+count_int64_len(cod->LEN_ADDR)+cod->LEN_INST+cod->LEN_ADDR+cod->LEN_DATA;
    write_integer(file, cod->LEN_CODE);
    write_integer(file, cod->TARGET_LEN);
    write_byte(file, cod->DELTA_INDICATOR);
    write_integer(file, cod->LEN_DATA);
    write_integer(file, cod->LEN_INST);
    write_integer(file, cod->LEN_ADDR);
}




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

static uint64_t addr_encode(addr_cache* cc, uint64_t addr, uint64_t here, uint8_t* mode){
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
uint32_t count_int64_len(uint64_t integer){
    uint32_t len=0;
    while(integer || !len){
        integer=integer>>7;
        len++;
    }
    return len;
}







/*测试用函数*/
D_RT test_output_delta(code *cod){
    FILE *delta=fopen("output", "wb+");
    /*WIN_HEADER*/
    fwrite(&cod->WIN_INDICATOR, sizeof(byte), 1, delta);
    write_integer(delta, cod->SOURCE_SEGMENT_LENGTH);
    write_integer(delta, cod->SOURCE_SEGMENT_POSITION);
    cod->LEN_CODE=count_int64_len(cod->TARGET_LEN)+count_int64_len(cod->DELTA_INDICATOR)+count_int64_len(cod->LEN_INST)+count_int64_len(cod->LEN_DATA)+count_int64_len(cod->LEN_ADDR)+cod->LEN_INST+cod->LEN_ADDR+cod->LEN_DATA;
    write_integer(delta, cod->LEN_CODE);
    write_integer(delta, cod->TARGET_LEN);
    fwrite(&cod->DELTA_INDICATOR, sizeof(byte), 1, delta);
    write_integer(delta, cod->LEN_DATA);
    write_integer(delta, cod->LEN_INST);
    write_integer(delta, cod->LEN_ADDR);
    
    
    /*WIN_CONTENT*/
    content_writer(delta,cod);
    fclose(delta);
    return D_OK;
}

void vcd_test(void){
    code *Code=(code *)malloc(sizeof(code));
    memset(Code, 0, sizeof(code));
    Code->WIN_INDICATOR=0x01;
    Code->SOURCE_SEGMENT_LENGTH=1024;
    Code->SOURCE_SEGMENT_POSITION=0;
    Code->TARGET_LEN=1024;
    instruction *Inst=create_instruction();
    instruction_node *new_nd= new_inst_node(NULL, COPY, 0, 10, NULL, 100);
    add_instructions(Inst, new_nd, 1);
    char buffer[]="HERE IS SOME DATA";
    instruction_node *new_nd2= new_inst_node(NULL, ADD, 10, sizeof(buffer), buffer,0);
    add_instructions(Inst, new_nd2, 1);
    code_instruction(Inst,Code);
    test_output_delta(Code);
}



void count_int_len_test(void){
    uint32_t test[3]={127,16383,2097151};
    for(int x=0;x<3;x++){
        printf("number %d length is %d\n",test[x],count_int64_len(test[x]));
    }
}
