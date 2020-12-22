//
//  SOURCE.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "SOURCE.h"


typedef struct _lru_manager{
    uint32_t BLOCK_COUNT;
    uint32_t BLOCK_IN_POOL;
    char source_window_pool[SOURCE_WINDOW_SIZE];
    block blk_in_pool[MAX_BLOCK_NUMBER];
    block *HEAD;
    block *TAIL;
    block *IN_POOL_BLOCK_HASH;
}lru_manager;

typedef enum _lru_mode{
    SINGLE,
    NORMAL
}lru_mode;

D_RT init_lru(lru_manager *lru,lru_mode mode,FILE *srcfile,size_t filesize){
    if(lru==NULL){
        printf("\nERROR: lru manager instance is NULL\n");
        return D_ERROR;
    }
    //lru->IN_POOL_BLOCK_HASH=NULL;
    if(mode==SINGLE){
        rewind(srcfile);
        size_t size=fread((void *)lru->source_window_pool,filesize , 1, srcfile);
        if(size!=filesize){
            printf("\nERROR: error during reading source file, mode=SINGLE\n");
            return D_ERROR;
        }
        lru->BLOCK_COUNT=1;
        lru->BLOCK_IN_POOL=1;
        lru->HEAD=&lru->blk_in_pool[0];
        lru->TAIL=&lru->blk_in_pool[0];
        lru->blk_in_pool[0].BLOCK_NUMBER=0;
        lru->blk_in_pool[0].DATA=lru->source_window_pool;
        lru->blk_in_pool[0].DATA_SIZE=filesize;
        lru->blk_in_pool[0].NEXT=NULL;
        lru->blk_in_pool[0].PREV=NULL;
        HASH_ADD_INT(lru->IN_POOL_BLOCK_HASH, BLOCK_NUMBER, &lru->blk_in_pool[0]);
    }else{
        rewind(srcfile);
        size_t size=fread((void *)lru->source_window_pool,SOURCE_WINDOW_SIZE , 1, srcfile);
        if(size!=SOURCE_WINDOW_SIZE){
            printf("\nERROR: error during reading source file, mode=SINGLE\n");
            return D_ERROR;
        }
        //round up 公式：int a = (b + (c - 1)) / c;
    }
    return D_OK;
    //还要设置current window
}








source *create_source(void){
    source *src=(source *)malloc(sizeof(source));
    memset(src, 0, sizeof(source));
    source_file *src_file=(source_file *)malloc(sizeof(source_file));
    memset(src_file, 0, sizeof(source_file));
    source_window *src_win=(source_window *)malloc(sizeof(source_window));
    memset(src_win, 0, sizeof(source_window));
    src_win->LRU_MANAGER=(lru_manager *)malloc(sizeof(lru_manager));
    memset(src_win->LRU_MANAGER, 0, sizeof(lru_manager));
    src_win->LRU_MANAGER->IN_POOL_BLOCK_HASH=NULL;
    src_win->WINDOW_SIZE=SOURCE_WINDOW_SIZE;
    src->SOURCE_FILE=src_file;
    src->SOURCE_WINDOW=src_win;
    return src;
}

D_RT set_file(source *src, const char *file_name){
    if(src==NULL){
        printf("\nERROR: source uninitilized\n");
        return D_ERROR;
    }else if(src->SOURCE_FILE==NULL){
        printf("\nERROR: source file uninitilized\n");
        return D_ERROR;
    }else if(file_name==NULL){
        printf("\nERROR: source file name cannot be empty\n");
        return D_ERROR;
    }
    FILE *srcfile=fopen(file_name, "r+b");
    fseek(srcfile,0,SEEK_END);
    size_t size=ftell(srcfile);
    rewind(srcfile);
    src->SOURCE_FILE->FILE_INSTANCE=srcfile;
    src->SOURCE_FILE->FILE_SIZE=size;
    src->SOURCE_FILE->FILE_NAME=file_name;
    return D_OK;
}


D_RT init_window(source *src){
    if(src==NULL){
        printf("\nERROR: source uninitialized\n");
        return D_ERROR;
    }else if(src->SOURCE_FILE->FILE_INSTANCE==NULL){
        printf("\nERROR: source file not opened yet\n");
        return D_ERROR;
    }
    size_t srcsize=src->SOURCE_FILE->FILE_SIZE;
    source_window *srcwin=src->SOURCE_WINDOW;
    lru_manager *lru=srcwin->LRU_MANAGER;
    
    if(srcsize<=SOURCE_WINDOW_SIZE){
        //srcwin->BLOCK_COUNT=1;
        //HASH_ADD_INT( lru->IN_POOL_BLOCK_HASH, id, s );
    }else{
        
        
        
        
    }
    return D_OK;
}

