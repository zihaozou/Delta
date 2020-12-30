//
//  type_def.h
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//

#ifndef type_def_h
#define type_def_h
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "uthash.h"
#include "crc16speed.h"


#define SOFTWARE_VERSION 0x01



//#include "utlist.h"
#define delta_min(a,b)      ((a)<(b))?(a):(b)
#define delta_max(a,b)      ((a)>(b))?(a):(b)




#define SOURCE_WINDOW_SIZE 4096
#define MAX_BLOCK_NUMBER 8
#define CRC_LEN 8
#define DEFAULT_TARGET_WIN_SIZE 2048
#define s_near 4
#define s_same 3
#if s_near<=0
#define s_near 4
#endif
#if s_same<=0
#define s_same 3
#endif
#define VCD_SELF 0
#define VCD_HERE 1



typedef unsigned char byte;
typedef enum delta_return{
    D_OK,
    D_ERROR,
    D_FULL,
    D_EMPTY,
    D_NONE,
    D_EXIST,
    D_NONEXIST
}D_RT;
typedef enum _inst_type{
    COPY,
    ADD,
    RUN,
    NOOP
}inst_type;
typedef union _data_addr{
    char *data;
    uint64_t addr;
}data_addr;
typedef enum _lru_mode{
    SINGLE,
    NORMAL
}lru_mode;







typedef struct _stream{
    uint64_t INPUT_REMAINING;
    uint64_t INPUT_POSITION;
    struct _target *TARGET;
    struct _source *SOURCE;
    //struct _target_window *CURRENT_WINDOW;
}stream;




typedef struct _target{
    struct _target_file *TARGET_FILE;
    struct _target_window *TARGET_WINDOW;
    //uint16_t WINDOW_COUNT;
}target;
typedef struct _target_file{
    FILE *FILE_INSTANCE;
    const char *FILE_NAME;
    size_t FILE_SIZE;
}target_file;
typedef struct _target_window{
    int WIN_NUMBER;
    char *BUFFER;
    size_t BUFFER_SIZE;
    uint64_t START_POSITION;//在目标文件中的起始位置
    uint64_t TOTAL_RAM_COST;//源文件片段+目标文件片段加起来的长度。
    struct _instruction *INSTRUCTION;
}target_window;





typedef struct _source{
    struct _source_file *SOURCE_FILE;
    struct _source_window *SOURCE_WINDOW;
    struct _source_hash *SOURCE_HASH;
}source;

typedef struct _source_file{
    FILE *FILE_INSTANCE;
    const char *FILE_NAME;
    size_t FILE_SIZE;
}source_file;

typedef struct _source_window{
    size_t WINDOW_SIZE;
    //uint32_t BLOCK_COUNT;
    struct _block *CURRENT_BLOCK;
    struct _lru_manager *LRU_MANAGER;
}source_window;

typedef struct _block{
    int BLOCK_NUMBER;
    char *DATA;
    size_t DATA_SIZE;
    struct _block *PREV;
    struct _block *NEXT;
    UT_hash_handle hh;
}block;

typedef struct _lru_manager{
    lru_mode MODE;
    uint64_t BLOCK_COUNT;
    uint32_t BLOCK_IN_POOL;
    char source_window_pool[SOURCE_WINDOW_SIZE];
    block blk_in_pool[MAX_BLOCK_NUMBER];
    block *HEAD;
    block *TAIL;
    block *IN_POOL_BLOCK_HASH;
}lru_manager;



typedef struct _instruction{
    uint8_t FIRST_COPY;
    uint32_t INSTRUCTION_COUNT;
    uint64_t START_POSITION;//在源文件中的起始位置
    uint64_t LENGTH;//指令在源文件中的跨度
    struct _instruction_node *HEAD;
    struct _instruction_node *TAIL;
}instruction;
typedef struct _instruction_node{
    struct _instruction_node *PREV;
    struct _instruction_node *NEXT;
    inst_type INST_TYPE;
    uint64_t POSITION;//此指令在目标文件中的起始位置
    uint64_t SIZE;
    data_addr DATA_or_ADDR;
}instruction_node;


typedef struct _source_hash{
    uint16_t crc;
    uint32_t cnt;
    struct _source_position *head;
    UT_hash_handle hh;
}source_hash;
typedef struct _source_position{
    struct _source_position *next;
    uint64_t position;
}source_position;



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
    
    
    struct _code_node *HEAD;
    struct _code_node *TAIL;
} code;
typedef struct _code_node{
    byte CODE;
    uint32_t SIZE1;
    uint32_t SIZE2;
    data_addr DATAADDR;
    uint32_t INST_SIZE;
    //uint32_t DATA_ADDR_SIZE;
    
    
    struct _code_node *NEXT;
    struct _code_node *PREV;
} code_node;

typedef struct _addr_cache{
    uint64_t near[s_near];
    int next_slot;
    uint64_t same[s_same*256];
} addr_cache;

#endif /* type_def_h */
