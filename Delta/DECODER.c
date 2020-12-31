//
//  DECODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#include "DECODER.h"
typedef size_t dsize;
typedef size_t dposition;




typedef struct _delta{
    FILE *FILE_INSTANCE;
    dsize FILE_SIZE;
    FILE *UPDATED_FILE;
    FILE *SOURCE_FILE;
    dsize SOURCE_SIZE;
    //byte TARGET_BUFFER[DEFAULT_TARGET_WIN_SIZE+DEFAULT_TARGET_WIN_SIZE/2];
    
    
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
    
    
    
    dposition CURRENT_WIN_POSI;
    dposition NEXT_WIN_POSI;
    dposition CURRENT_DATA_POSI;
    dposition CURRENT_INST_POSI;
    dposition CURRENT_ADDR_POSI;
}delta;


delta *create_delta(const char *delta_name,const char *updated_name,const char *source_name);
void set_win_header(delta *del);
D_RT verify_file(delta *Del);






void set_HDR(delta *del){
    fseek(del->FILE_INSTANCE, 4, SEEK_SET);
    del->HDR_INDICATOR=getc(del->FILE_INSTANCE);
    return;
}
void set_first_win(delta *del){
    //如果要使用二次压缩，那这里就需要修改
    fseek(del->FILE_INSTANCE, 5, SEEK_SET);
    del->CURRENT_WIN_POSI=ftell(del->FILE_INSTANCE);
    del->NEXT_WIN_POSI=0;
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
    addr_cache cache;
    cache_init(&cache);
    FILE *delfile=del->FILE_INSTANCE;
    if(del->CURRENT_WIN_POSI!=ftell(delfile))return D_ERROR;
    set_win_header(del);
    //memset(del->TARGET_BUFFER, 0, DEFAULT_TARGET_WIN_SIZE+DEFAULT_TARGET_WIN_SIZE/2);
    if(del->SOURCE_SEGMENT_LENGTH>del->SOURCE_SIZE){
        printf("\nERROR source segment size larger than source size\n");
        return D_ERROR;
    }else if(del->SOURCE_SEGMENT_POSITION>=del->SOURCE_SIZE){
        printf("\nERROR source segment position exceeds the source size\n");
        return D_ERROR;
    }
    dposition curr_decode_position=del->SOURCE_SEGMENT_LENGTH;
    byte *merged_buffer=(byte *)malloc(sizeof(byte)*(del->SOURCE_SEGMENT_LENGTH+del->LENGTH_TARGET_WIN));
    //curr_decode_position=&merged_buffer[del->SOURCE_SEGMENT_LENGTH];
    fseek(del->SOURCE_FILE, del->SOURCE_SEGMENT_POSITION, SEEK_SET);
    size_t read_size=fread(merged_buffer, 1, del->SOURCE_SEGMENT_LENGTH, del->SOURCE_FILE);
    if(read_size!=del->SOURCE_SEGMENT_LENGTH){
        printf("\nERROR during reading source segment\n");
        return D_ERROR;
    }
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
                    addr=curr_decode_position-read_integer_at(delfile, addr_posi);
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
                memcpy(&merged_buffer[curr_decode_position], &merged_buffer[addr], size);
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
                read_bytes_at(delfile, size, data_posi, &merged_buffer[curr_decode_position]);
                data_posi=ftell(delfile);
                curr_decode_position+=size;
                break;
            case RUN:
                //设置size
                size=read_integer_at(delfile, inst_posi);
                inst_posi=ftell(delfile);
                memset(&merged_buffer[curr_decode_position], read_byte_at(delfile, data_posi), size);
                data_posi=ftell(delfile);
                curr_decode_position+=size;
                break;
            default:
                break;
        }
    }
    fwrite(&merged_buffer[del->SOURCE_SEGMENT_LENGTH], del->LENGTH_TARGET_WIN, 1, del->UPDATED_FILE);
    free(merged_buffer);
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

void DECODER(const char *delta_name,const char *source_name,const char *updated_name){
    delta *Del=create_delta(delta_name,updated_name,source_name);
    if(verify_file(Del)==D_ERROR){
        printf("\nERROR delta file incorrect\n");
        exit(0);
    }
    set_HDR(Del);
    set_first_win(Del);
    while(Del->NEXT_WIN_POSI<Del->FILE_SIZE){
        decode_window(Del);
    }
    fclose(Del->UPDATED_FILE);
    
}

delta *create_delta(const char *delta_name,const char *updated_name,const char *source_name){
    delta *Delta=(delta *)malloc(sizeof(delta));
    Delta->FILE_INSTANCE=fopen(delta_name, "rb+");
    if(Delta->FILE_INSTANCE==NULL){
        printf("\nERROR during reading delta file\n");
        exit(0);
    }
    fseek(Delta->FILE_INSTANCE, 0, SEEK_END);
    Delta->FILE_SIZE=(dsize)ftell(Delta->FILE_INSTANCE);
    rewind(Delta->FILE_INSTANCE);
    Delta->UPDATED_FILE=fopen(updated_name, "wb+");
    Delta->SOURCE_FILE=fopen(source_name, "rb+");
    if(Delta->SOURCE_FILE==NULL){
        printf("\nERROR during reading source file\n");
        exit(0);
    }
    fseek(Delta->SOURCE_FILE, 0, SEEK_END);
    Delta->SOURCE_SIZE=(dsize)ftell(Delta->SOURCE_FILE);
    rewind(Delta->SOURCE_FILE);
    return Delta;
}


D_RT verify_file(delta *Del){
    byte temp;
    FILE *delfile=Del->FILE_INSTANCE;
    fread(&temp, 1, 1, delfile);
    if(temp!=0xd6)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    if(temp!=0xc3)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    if(temp!=0xc4)return D_ERROR;
    fread(&temp, 1, 1, delfile);
    rewind(Del->FILE_INSTANCE);
    if(temp!=SOFTWARE_VERSION)return D_ERROR;
    return D_OK;
}
