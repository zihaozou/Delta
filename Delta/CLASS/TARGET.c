//
//  TARGET.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "TARGET.h"

target *create_target(void){
    target *tgt=(target *)malloc(sizeof(target));
    tgt->TARGET_FILE=(target_file *)malloc(sizeof(target_file));
    tgt->TARGET_WINDOW=(target_window *)malloc(sizeof(target_window));
    //tgt->WINDOW_COUNT=0;
    memset(tgt->TARGET_FILE, 0, sizeof(target_file));
    memset(tgt->TARGET_WINDOW,0,sizeof(target_window));
    tgt->TARGET_WINDOW->BUFFER=(char *)malloc(DEFAULT_TARGET_WIN_SIZE*sizeof(char));
    tgt->TARGET_WINDOW->WIN_NUMBER=-1;
    return tgt;
}
D_RT set_tgt_file(target *tgt,const char *filename){
    if(tgt==NULL){
        printf("\nERROR: target uninitilized\n");
        return D_ERROR;
    }else if(tgt->TARGET_FILE==NULL){
        printf("\nERROR: target file uninitilized\n");
        return D_ERROR;
    }else if(filename==NULL){
        printf("\nERROR: target file name cannot be empty\n");
        return D_ERROR;
    }
    FILE *tgtfile=fopen(filename, "r+b");
    fseek(tgtfile,0,SEEK_END);
    size_t size=ftell(tgtfile);
    rewind(tgtfile);
    tgt->TARGET_FILE->FILE_INSTANCE=tgtfile;
    tgt->TARGET_FILE->FILE_NAME=filename;
    tgt->TARGET_FILE->FILE_SIZE=size;
    return D_OK;
}

D_RT set_window(target *tgt){
    if(tgt==NULL){
        printf("\nERROR: target uninitilized\n");
        return D_ERROR;
    }else if(tgt->TARGET_WINDOW==NULL){
        printf("\nERROR: target window uninitilized\n");
        return D_ERROR;
    }else if(tgt->TARGET_FILE==NULL){
        printf("\nERROR: target file uninitilized\n");
        return D_ERROR;
    }else if(tgt->TARGET_FILE->FILE_INSTANCE==NULL){
        printf("\nERROR: target file instance unloaded\n");
        return D_ERROR;
    }
#define FIXED_WINDOW_DIVIDE
#if defined FIXED_WINDOW_DIVIDE
    //使用固定的窗口切分方式
    target_window *tgtwin=tgt->TARGET_WINDOW;
    FILE *tgtf=tgt->TARGET_FILE->FILE_INSTANCE;
    memset(tgtwin->BUFFER, 0, DEFAULT_TARGET_WIN_SIZE*sizeof(char));
    tgtwin->START_POSITION=ftell(tgtf);
    if(tgtwin->START_POSITION>=tgt->TARGET_FILE->FILE_SIZE)return D_EMPTY;//源文件已读完
    size_t data_remain=tgt->TARGET_FILE->FILE_SIZE-tgtwin->START_POSITION;
    
    if(data_remain>=DEFAULT_TARGET_WIN_SIZE){
        size_t size=fread((void *)tgtwin->BUFFER, 1, DEFAULT_TARGET_WIN_SIZE, tgtf);
        if(size!=DEFAULT_TARGET_WIN_SIZE){
            printf("\nERROR: error during reading target file in set window\n");
            return D_ERROR;
        }
        tgtwin->BUFFER_SIZE=DEFAULT_TARGET_WIN_SIZE;
    }else{
        size_t size=fread((void *)tgtwin->BUFFER, 1, data_remain, tgtf);
        if(size!=data_remain){
            printf("\nERROR: error during reading target file in set window\n");
            return D_ERROR;
        }
        tgtwin->BUFFER_SIZE=data_remain;
    }
    tgtwin->TOTAL_RAM_COST=tgtwin->BUFFER_SIZE;
    if(tgtwin->INSTRUCTION!=NULL){
        delete_inst_list(tgtwin->INSTRUCTION);
    }
    tgtwin->INSTRUCTION=create_instruction();
    tgtwin->WIN_NUMBER++;
#else
    //可能使用其他切分方式
#endif
    
    
    return D_OK;
}




