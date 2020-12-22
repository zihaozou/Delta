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

#define delta_min(a,b)      (a<b)?a:b
#define delta_max(a,b)      (a>b)?a:b




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
    RUN
}inst_type;
typedef union _data_addr{
    char *data;
    uint32_t addr;
}data_addr;








typedef struct _stream{
    uint32_t INPUT_REMAINING;
    uint32_t INPUT_POSITION;
    struct _target *TARGET;
    struct _source *SOURCE;
    struct _target_window *CURRENT_WINDOW;
}stream;




typedef struct _target{
    struct _target_file *TARGET_FILE;
    struct _target_window **target_window_list;
    uint32_t WINDOW_COUNT;
}target;
typedef struct _target_file{
    FILE *FILE_INSTANCE;
    char *FILE_NAME;
    size_t FILE_SIZE;
}target_file;
typedef struct _target_window{
    uint32_t WIN_NUMBER;
    char *BUFFER;
    size_t BUFFER_SIZE;
    uint32_t START_POSITION;
    struct _instruction *INSTRUCTION;
}target_window;





typedef struct _source{
    struct _source_file *source_file;
    struct _source_window *source_window;
    //hash table
}source;

typedef struct _source_file{
    FILE *FILE_INSTANCE;
    char *FILE_NAME;
    size_t *FILE_SIZE;
}source_file;

typedef struct _source_window{
    size_t WINDOW_SIZE;
    uint32_t BLOCK_COUNT;
    struct _block *CURRENT_BLOCK;
    struct _lru_manager *LRU_MANAGER;
}source_window;

typedef struct _block{
    uint32_t BLOCK_NUMBER;
    char *DATA;
    size_t DATA_SIZE;
}block;

typedef struct _instruction{
    uint32_t instruction_count;
    struct _instruction *HEAD;
    struct _instruction *TAIL;
}instruction;
typedef struct _instruction_node{
    struct _instruction_node *PREV;
    struct _instruction_node *NEXT;
    inst_type INST_TYPE;
    uint32_t SIZE;
    data_addr DATA_or_ADDR;
}instruction_node;




#endif /* type_def_h */
