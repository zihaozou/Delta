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
//  DECODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#include "DECODER.h"
typedef size_t dsize;
typedef size_t dposition;



#define DECODE_SOURCE_SIZE 1024
#define DECODE_BLOCK_NUMBER 4
typedef struct _decode_block{
    int BLOCK_NUMBER;
    byte *DATA;
    size_t DATA_SIZE;
    struct _decode_block *PREV;
    struct _decode_block *NEXT;
}decode_block;
typedef struct _delta{
    int DECODE_MODE;
    FILE *FILE_INSTANCE;
    dsize FILE_SIZE;
    FILE *UPDATED_FILE;
    dsize UPDATED_SIZE;
    FILE *SOURCE_FILE;
    dsize SOURCE_SIZE;
    byte *UPDATED_BUFFER;
    int UPDATED_WIN_SIZE;
    
    dsize SOURCE_WIN_SIZE;
    byte SOURCE_BUFFER[DECODE_SOURCE_SIZE];
    decode_block DECODE_BLOCK_LIST[DECODE_BLOCK_NUMBER];
    decode_block *CURRENT_BLOCK;
    decode_block *HEAD;
    decode_block *TAIL;
    uint32_t BLOCK_COUNT;
    uint8_t BLOCK_INPOOL;
    
    byte HDR_INDICATOR;
    //暂时没有二次压缩和自定义表
    //window
    byte WIN_INDICATOR;
    dsize SOURCE_SEGMENT_LENGTH;
    dposition SOURCE_SEGMENT_POSITION;
    //delta encode
    dsize LENGTH_DELTA_ENCODE;
    dsize LENGTH_TARGET_WIN;
    byte DELTA_INDICATOR;
    dsize LEN_DATA;
    dsize LEN_INST;
    dsize LEN_ADDR;
    
    dposition TARGET_POSI;
    
    dposition CURRENT_WIN_POSI;
    dposition NEXT_WIN_POSI;
    dposition CURRENT_DATA_POSI;
    dposition CURRENT_INST_POSI;
    dposition CURRENT_ADDR_POSI;
}delta;


void init_delta(delta *del,const char *delta_name,const char *updated_name,const char *source_name);
void set_win_header(delta *del);
D_RT verify_file(delta *Del);

D_RT init_source_lru(delta *del){
    if(del->SOURCE_SEGMENT_LENGTH<=DECODE_SOURCE_SIZE){
        fseek(del->SOURCE_FILE, del->SOURCE_SEGMENT_POSITION, SEEK_SET);
        size_t rsize=fread(del->SOURCE_BUFFER, 1, del->SOURCE_SEGMENT_LENGTH,del->SOURCE_FILE);
        if(rsize!=del->SOURCE_SEGMENT_LENGTH){
            printf("\nERROR: during read source segment\n");
            return D_ERROR;
        }
        del->BLOCK_COUNT=1;
        del->BLOCK_INPOOL=1;
        del->HEAD=&del->DECODE_BLOCK_LIST[0];
        del->TAIL=&del->DECODE_BLOCK_LIST[0];
        del->DECODE_BLOCK_LIST[0].BLOCK_NUMBER=0;
        del->DECODE_BLOCK_LIST[0].DATA=del->SOURCE_BUFFER;
        del->DECODE_BLOCK_LIST[0].DATA_SIZE=del->SOURCE_SEGMENT_LENGTH;
        del->DECODE_BLOCK_LIST[0].NEXT=NULL;
        del->DECODE_BLOCK_LIST[0].PREV=NULL;
        //del->SOURCE_WIN_SIZE=del->SOURCE_SEGMENT_LENGTH;
    }else{
        fseek(del->SOURCE_FILE, del->SOURCE_SEGMENT_POSITION, SEEK_SET);
        size_t rsize=fread(del->SOURCE_BUFFER,1 , DECODE_SOURCE_SIZE,del->SOURCE_FILE);
        if(rsize!=DECODE_SOURCE_SIZE){
            printf("\nERROR: during read source segment\n");
            return D_ERROR;
        }
        uint64_t init_block_size=DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER;
        //a = (b + (c - 1)) / c
        del->BLOCK_COUNT=(uint32_t)(del->SOURCE_SEGMENT_LENGTH+(init_block_size-1))/init_block_size;
        del->BLOCK_INPOOL=DECODE_BLOCK_NUMBER;
        del->HEAD=&del->DECODE_BLOCK_LIST[0];
        del->TAIL=&del->DECODE_BLOCK_LIST[DECODE_BLOCK_NUMBER-1];
        for(int x=0;x<DECODE_BLOCK_NUMBER;x++){//设置每个block
            del->DECODE_BLOCK_LIST[x].BLOCK_NUMBER=x;
            del->DECODE_BLOCK_LIST[x].DATA=&del->SOURCE_BUFFER[x*init_block_size];
            del->DECODE_BLOCK_LIST[x].DATA_SIZE=init_block_size;
            if(x==0){
                del->DECODE_BLOCK_LIST[x].PREV=NULL;
                del->DECODE_BLOCK_LIST[x].NEXT=&del->DECODE_BLOCK_LIST[x+1];
            }else if(x==DECODE_BLOCK_NUMBER-1){
                del->DECODE_BLOCK_LIST[x].PREV=&del->DECODE_BLOCK_LIST[x-1];
                del->DECODE_BLOCK_LIST[x].NEXT=NULL;
            }else{
                del->DECODE_BLOCK_LIST[x].PREV=&del->DECODE_BLOCK_LIST[x-1];
                del->DECODE_BLOCK_LIST[x].NEXT=&del->DECODE_BLOCK_LIST[x+1];
            }
            //HASH_ADD_INT(lru->IN_POOL_BLOCK_HASH, BLOCK_NUMBER, &lru->blk_in_pool[x]);
        }
    }
    del->CURRENT_BLOCK=&del->DECODE_BLOCK_LIST[0];
    
    return D_OK;
}
void decode_put_tail_to_head(delta *del,decode_block *blk){
    blk->PREV->NEXT=NULL;
    del->TAIL=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=del->HEAD;
    del->HEAD->PREV=blk;
    del->HEAD=blk;
    return;
}

void decode_put_mid_to_head(delta *del,decode_block *blk){
    blk->PREV->NEXT=blk->NEXT;
    blk->NEXT->PREV=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=del->HEAD;
    del->HEAD->PREV=blk;
    del->HEAD=blk;
    return;
}

decode_block *find_block_in_pool(delta *del,uint32_t blk_no){
    for(int x=0;x<del->BLOCK_INPOOL;x++){
        if(del->DECODE_BLOCK_LIST[x].BLOCK_NUMBER==blk_no)return &del->DECODE_BLOCK_LIST[x];
    }
    return NULL;
}

D_RT decode_get_block(delta *del,uint32_t blk_no){
    if(del->CURRENT_BLOCK->BLOCK_NUMBER==blk_no)return D_OK;
    decode_block *temp=find_block_in_pool(del, blk_no);
    if (temp==NULL) {
        size_t data_size;
        uint32_t data_position=(uint32_t)del->SOURCE_SEGMENT_POSITION+(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER)*blk_no;
        temp=del->TAIL;
        decode_put_tail_to_head(del,temp);
        temp->BLOCK_NUMBER=blk_no;
        if(blk_no==del->BLOCK_COUNT-1 && del->SOURCE_SEGMENT_LENGTH%(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER)!=0){
            data_size=del->SOURCE_SEGMENT_LENGTH%(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
        }else data_size=DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER;
        memset(temp->DATA, 0, DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
        fseek(del->SOURCE_FILE, data_position, SEEK_SET);
        size_t size=fread(temp->DATA, 1, data_size, del->SOURCE_FILE);
        if(size!=data_size){
            printf("\nERROR: error during reading source file in get block, block number=%d\n",blk_no);
            return D_ERROR;
        }
        temp->DATA_SIZE=data_size;
        del->CURRENT_BLOCK=temp;
    } else {
        if(temp==del->TAIL){
            decode_put_tail_to_head(del,temp);
        }else{
            decode_put_mid_to_head(del, temp);
        }
        del->CURRENT_BLOCK=temp;
    }
    
    return D_OK;
}

D_RT copy_data(delta *del,dposition decode_posi, dposition addr,dsize siz){
    uint64_t init_blk_size;
    uint32_t blk_no;
    uint32_t local_position;
    dsize data_remain;
    if(addr>=del->SOURCE_SEGMENT_LENGTH){//scopy
        for (int x=0; x<siz; x++) {
            del->UPDATED_BUFFER[decode_posi+x]=del->UPDATED_BUFFER[addr-del->SOURCE_SEGMENT_LENGTH+x];
        }
    }else{
        init_blk_size=(del->BLOCK_COUNT==1)?del->SOURCE_SEGMENT_LENGTH:(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
        blk_no=(uint32_t)addr/(init_blk_size);
        local_position=(uint32_t)addr%(init_blk_size);
        while (siz) {//copy
            decode_get_block(del, blk_no);
            data_remain=del->CURRENT_BLOCK->DATA_SIZE-local_position;
            if(data_remain>=siz){
                memcpy(&del->UPDATED_BUFFER[decode_posi], &del->CURRENT_BLOCK->DATA[local_position], siz);
                siz=0;
            }else{
                memcpy(&del->UPDATED_BUFFER[decode_posi], &del->CURRENT_BLOCK->DATA[local_position], data_remain);
                siz-=data_remain;
                blk_no+=1;
                local_position=0;
                decode_posi+=data_remain;
            }
        }
    }
    
    return D_OK;
}
void set_HDR(delta *del){
    del->HDR_INDICATOR=getc(del->FILE_INSTANCE);
    if(del->HDR_INDICATOR&(1<<2))del->DECODE_MODE=1;
    else if(del->HDR_INDICATOR&(1<<3))del->DECODE_MODE=2;
    else del->DECODE_MODE=3;
    return;
}
void set_first_win(delta *del){
    //如果要使用二次压缩，那这里就需要修改
    del->CURRENT_WIN_POSI=ftell(del->FILE_INSTANCE);
    del->NEXT_WIN_POSI=0;
    del->TARGET_POSI=0;
    return;
}
uint64_t read_integer(FILE *f){
    uint64_t integer=0;
    byte temp=0;
    do {
        integer<<=7;
        temp=getc(f);
        integer|=(temp & (~0x80));
    } while (temp & 0x80);
    return integer;
}
uint64_t read_integer_at(FILE *f,dposition location){
    fseek(f, location, SEEK_SET);
    return read_integer(f);
}
byte read_byte(FILE *f){
    return getc(f);
}
void read_bytes_at(FILE *f,dsize s,dposition p,byte *b){
    fseek(f, p, SEEK_SET);
    fread(b, sizeof(byte), s, f);
    return;
}
byte read_byte_at(FILE *f,dposition location){
    fseek(f, location, SEEK_SET);
    return getc(f);
}
D_RT decode_window(delta *del){
    uint32_t add_size=0;
    addr_cache cache;
    cache_init(&cache);
    FILE *delfile=del->FILE_INSTANCE;
    if(del->CURRENT_WIN_POSI!=ftell(delfile))return D_ERROR;
    set_win_header(del);
    init_source_lru(del);//初始化lru
    memset(del->UPDATED_BUFFER, 0, del->UPDATED_WIN_SIZE);
    dposition curr_decode_position=0;
    //reading delta routine
    dposition inst_posi=del->CURRENT_INST_POSI;
    dposition addr_posi=del->CURRENT_ADDR_POSI;
    dposition data_posi=del->CURRENT_DATA_POSI;
    byte instcode,mode;
    int m;
    dposition addr;
    dsize size;
    while(inst_posi<del->CURRENT_ADDR_POSI){
        instcode=read_byte_at(delfile, inst_posi);
        inst_posi++;
        switch (DeltaDefaultCodeTable[0][instcode]) {
            case COPY:
                //设置size
                if(DeltaDefaultCodeTable[2][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=ftell(delfile);
                }else{
                    size=DeltaDefaultCodeTable[2][instcode];
                }
                //设置addr
                mode=DeltaDefaultCodeTable[4][instcode];
                if(mode==VCD_SELF){
                    addr=read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else if (mode==VCD_HERE){
                    addr=curr_decode_position+del->SOURCE_SEGMENT_LENGTH-read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else if((m=mode-2)>=0 && m<s_near){
                    addr=cache.near[m]+read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else{
                    m=mode-(2+s_near);
                    addr=cache.same[m*256+read_byte_at(delfile, addr_posi)];
                    addr_posi=ftell(delfile);
                }
                cache_update(&cache, addr);
                //读取数据
                copy_data(del, curr_decode_position, addr, size);
                curr_decode_position+=size;
                break;
            case ADD:
                //设置size
                if(DeltaDefaultCodeTable[2][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=ftell(delfile);
                }else{
                    size=DeltaDefaultCodeTable[2][instcode];
                }
                //读取数据
                read_bytes_at(delfile, size, data_posi, &del->UPDATED_BUFFER[curr_decode_position]);
                data_posi=ftell(delfile);
                curr_decode_position+=size;
                add_size+=size;
                break;
            case RUN:
                //设置size
                size=read_integer_at(delfile, inst_posi);
                inst_posi=ftell(delfile);
                memset(&del->UPDATED_BUFFER[curr_decode_position], read_byte_at(delfile, data_posi), size);
                data_posi=ftell(delfile);
                curr_decode_position+=size;
                break;
            default:
                break;
        }
        switch (DeltaDefaultCodeTable[1][instcode]) {
            case COPY:
                if(DeltaDefaultCodeTable[3][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=ftell(delfile);
                }else{
                    size=DeltaDefaultCodeTable[3][instcode];
                }
                //设置addr
                mode=DeltaDefaultCodeTable[5][instcode];
                if(mode==VCD_SELF){
                    addr=read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else if (mode==VCD_HERE){
                    addr=curr_decode_position+del->SOURCE_SEGMENT_LENGTH-read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else if((m=mode-2)>=0 && m<s_near){
                    addr=cache.near[m]+read_integer_at(delfile, addr_posi);
                    addr_posi=ftell(delfile);
                }else{
                    m=mode-(2+s_near);
                    addr=cache.same[m*256+read_byte_at(delfile, addr_posi)];
                    addr_posi=ftell(delfile);
                }
                cache_update(&cache, addr);
                //读取数据
                copy_data(del, curr_decode_position, addr, size);
                curr_decode_position+=size;
                break;
            case ADD:
                //设置size
                if(DeltaDefaultCodeTable[3][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=ftell(delfile);
                }else{
                    size=DeltaDefaultCodeTable[3][instcode];
                }
                //读取数据
                read_bytes_at(delfile, size, data_posi, &del->UPDATED_BUFFER[curr_decode_position]);
                data_posi=ftell(delfile);
                curr_decode_position+=size;
                add_size+=size;
                break;
            default:
                break;
        }
    }
    if(del->DECODE_MODE!=1){
        fseek(del->SOURCE_FILE, del->TARGET_POSI, SEEK_SET);
        fwrite(del->UPDATED_BUFFER, del->LENGTH_TARGET_WIN, 1, del->SOURCE_FILE);
        del->SOURCE_SIZE=delta_max(del->SOURCE_SIZE, del->TARGET_POSI+del->LENGTH_TARGET_WIN);
        del->TARGET_POSI+=del->LENGTH_TARGET_WIN;
    }
    fwrite(del->UPDATED_BUFFER, del->LENGTH_TARGET_WIN, 1, del->UPDATED_FILE);
    //free(merged_buffer);
    fseek(delfile,del->NEXT_WIN_POSI,SEEK_SET);
    del->CURRENT_WIN_POSI=del->NEXT_WIN_POSI;
    return D_OK;
}

void set_win_header(delta *del){
    FILE *delfile=del->FILE_INSTANCE;
    del->WIN_INDICATOR=read_byte(delfile);
    del->SOURCE_SEGMENT_LENGTH=read_integer(delfile);
    del->SOURCE_SEGMENT_POSITION=read_integer(delfile);
    del->LENGTH_DELTA_ENCODE=read_integer(delfile);
    del->NEXT_WIN_POSI=ftell(delfile)+del->LENGTH_DELTA_ENCODE;
    del->LENGTH_TARGET_WIN=read_integer(delfile);
    del->DELTA_INDICATOR=read_byte(delfile);
    del->LEN_DATA=read_integer(delfile);
    del->LEN_INST=read_integer(delfile);
    del->LEN_ADDR=read_integer(delfile);
    del->CURRENT_DATA_POSI=ftell(delfile);
    del->CURRENT_INST_POSI=del->CURRENT_DATA_POSI+del->LEN_DATA;
    del->CURRENT_ADDR_POSI=del->CURRENT_INST_POSI+del->LEN_INST;
}
void test_read_integer(void){
    FILE *f=fopen("test_read_integer", "rb+");
    uint64_t integer=0;
    byte temp=0;
    do {
        integer<<=7;
        temp=getc(f);
        integer|=(temp & (~0x80));
    } while (temp & 0x80);
}





static delta Del;
void DECODER(const char *delta_name,const char *source_name,const char *updated_name, int page_size){
    //delta *Del=create_delta(delta_name,updated_name,source_name);
    FILE *temp;
    uint64_t add_size;
    init_delta(&Del, delta_name, updated_name, source_name);
    Del.UPDATED_WIN_SIZE=page_size;
    Del.UPDATED_BUFFER=(byte *)malloc(sizeof(byte)*page_size);
    if(verify_file(&Del)==D_ERROR){
        printf("\nERROR: delta file incorrect\n");
        exit(0);
    }
    Del.UPDATED_SIZE=read_integer(Del.FILE_INSTANCE);
    set_HDR(&Del);
    if(Del.DECODE_MODE!=1){
        temp=fopen("temp.bin", "wb+");
        byte *tempbuffer=(byte *)malloc(sizeof(byte)*Del.SOURCE_SIZE);
        fread(tempbuffer, 1, Del.SOURCE_SIZE, Del.SOURCE_FILE);
        fwrite(tempbuffer, Del.SOURCE_SIZE, 1, temp);
        fclose(Del.SOURCE_FILE);
        Del.SOURCE_FILE=temp;
        rewind(temp);
        free(tempbuffer);
    }
    if(Del.DECODE_MODE==3){
        add_size=read_integer(Del.FILE_INSTANCE);
        byte *temp=(void *)malloc(sizeof(byte)*(Del.SOURCE_SIZE+Del.UPDATED_SIZE));
        memset(temp, 0, sizeof(byte)*(Del.SOURCE_SIZE+Del.UPDATED_SIZE));
        fseek(Del.SOURCE_FILE, 0, SEEK_SET);
        fread(&temp[add_size], 1, Del.SOURCE_SIZE, Del.SOURCE_FILE);
        //memmove(&temp[add_size], &temp[0], Del.SOURCE_SIZE);
        //memcpy(&temp[0], Del.UPDATED_BUFFER, Del.LENGTH_TARGET_WIN);
        Del.SOURCE_SIZE=Del.SOURCE_SIZE+add_size;
        fseek(Del.SOURCE_FILE, 0, SEEK_SET);
        fwrite(temp, Del.SOURCE_SIZE, 1, Del.SOURCE_FILE);
        //Del.TARGET_POSI+=Del.LENGTH_TARGET_WIN;
        free(temp);
    }
    set_first_win(&Del);
    while(Del.NEXT_WIN_POSI<Del.FILE_SIZE){
        decode_window(&Del);
    }
    fclose(Del.SOURCE_FILE);
    fclose(Del.FILE_INSTANCE);
    fclose(Del.UPDATED_FILE);
    if(Del.DECODE_MODE!=1)remove("temp.bin");
}

void init_delta(delta *del,const char *delta_name,const char *updated_name,const char *source_name){
    //delta *Delta=(delta *)malloc(sizeof(delta));
    del->FILE_INSTANCE=fopen(delta_name, "rb+");
    if(del->FILE_INSTANCE==NULL){
        printf("\nERROR: during reading delta file\n");
        exit(0);
    }
    fseek(del->FILE_INSTANCE, 0, SEEK_END);
    del->FILE_SIZE=(dsize)ftell(del->FILE_INSTANCE);
    rewind(del->FILE_INSTANCE);
    del->UPDATED_FILE=fopen(updated_name, "wb+");
    del->SOURCE_FILE=fopen(source_name, "rb+");
    if(del->SOURCE_FILE==NULL){
        printf("\nERROR: during reading source file\n");
        exit(0);
    }
    fseek(del->SOURCE_FILE, 0, SEEK_END);
    del->SOURCE_SIZE=(dsize)ftell(del->SOURCE_FILE);
    rewind(del->SOURCE_FILE);
    return;
}


D_RT verify_file(delta *Del){
    uint8_t check_buff[2048];
    uint8_t x;
    uint8_t md5_from_delta[16];
    uint8_t md5_calculated[16];
    uint32_t read_size;
    MD5_CTX md5_checker;
    uint32_t calculated=0;
    byte temp;
    FILE *delfile=Del->FILE_INSTANCE;
    for (x=0;x<16;x++){
        md5_from_delta[x]=read_byte(delfile);
    }
    uint32_t file_size=(read_byte(delfile)<<8)|read_byte(delfile);
    MD5Init(&md5_checker);
    while(calculated<file_size){
        read_size=delta_min(file_size-calculated,2048);
        fread((void *)&check_buff[0], 1, read_size, delfile);
        MD5Update(&md5_checker,&check_buff[0],read_size);
//        MD5Final(md5_calculated, &md5_checker);
        calculated+=read_size;
    }
    MD5Final(md5_calculated, &md5_checker);
    for(x=0;x<16;x++){
        if(md5_calculated[x]!=md5_from_delta[x]){
            return D_ERROR;
        }
    }
    //Del->FILE_SIZE=file_size;
    fseek(delfile, 18, SEEK_SET);
    MD5Init(&md5_checker);
    for (x=0;x<16;x++){
        md5_from_delta[x]=read_byte(delfile);
    }
    file_size=(uint32_t)read_integer(delfile);
    calculated=0;
    while(calculated<file_size){
        read_size=delta_min(file_size-calculated,2048);
        fread((void *)&check_buff[0], 1, read_size, Del->SOURCE_FILE);
        MD5Update(&md5_checker,&check_buff[0],read_size);
        calculated+=read_size;
    }
    MD5Final(md5_calculated, &md5_checker);
    for(x=0;x<16;x++){
        if(md5_calculated[x]!=md5_from_delta[x]){
            return D_ERROR;
        }
    }
    rewind(Del->SOURCE_FILE);
    fread(&temp, 1, 1, delfile);
    if(temp!=0xd6)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    if(temp!=0xc3)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    if(temp!=0xc4)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    //rewind(Del->FILE_INSTANCE);
    if(temp!=SOFTWARE_VERSION)return D_ERROR;
    return D_OK;
}
