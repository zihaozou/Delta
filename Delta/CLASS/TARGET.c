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
//  TARGET.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "TARGET.h"

target *create_target(int page_size){
    target *tgt=(target *)malloc(sizeof(target));
    tgt->TARGET_FILE=(target_file *)malloc(sizeof(target_file));
    tgt->TARGET_WINDOW=(target_window *)malloc(sizeof(target_window));
    //tgt->WINDOW_COUNT=0;
    memset(tgt->TARGET_FILE, 0, sizeof(target_file));
    memset(tgt->TARGET_WINDOW,0,sizeof(target_window));
    tgt->TARGET_WINDOW->BUFFER=(char *)malloc((page_size)*sizeof(char));
    tgt->TARGET_WINDOW->WIN_NUMBER=-1;
    tgt->TARGET_WINDOW->WIN_SIZE=page_size;
    return tgt;
}
D_RT clean_target(target *tgt){
    free(tgt->TARGET_WINDOW->BUFFER);
    free(tgt->TARGET_WINDOW);
    fclose(tgt->TARGET_FILE->FILE_INSTANCE);
    free(tgt->TARGET_FILE);
    free(tgt);
    return D_OK;
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
    memset(tgtwin->BUFFER, 0, (tgtwin->WIN_SIZE)*sizeof(char));
    tgtwin->START_POSITION=ftell(tgtf);
    if(tgtwin->START_POSITION>=tgt->TARGET_FILE->FILE_SIZE)return D_EMPTY;//源文件已读完
    size_t data_remain=tgt->TARGET_FILE->FILE_SIZE-tgtwin->START_POSITION;
    
    if(data_remain>=(tgtwin->WIN_SIZE)){
        size_t size=fread((void *)tgtwin->BUFFER, 1, tgtwin->WIN_SIZE, tgtf);
        if(size!=tgtwin->WIN_SIZE){
            printf("\nERROR: error during reading target file in set window\n");
            return D_ERROR;
        }
        tgtwin->BUFFER_SIZE=tgtwin->WIN_SIZE;
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




