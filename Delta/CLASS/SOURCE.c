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
//  SOURCE.c
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#include "SOURCE.h"

///LRU_MANAGER

/// init_lru:
/// 这个函数初始化lru队列，在这里我简单讲一下lru工作原理：lru由一个双向链表和一个哈希表组成。链表遵循FIFO原理，新进的BLO
/// CK被放到最前列，最近未使用的block被推向最后列。哈希表则记录链表中的元素的位置，以达到O(1) 的查询时间。
/// 如果源文件大小小于源窗口大小，则把整个源文件作为一整个block放入队列中
/// 如果源文件大于源窗口大小，则把源文件分为一个个block，block大小为SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER（最后
/// 一个block除外，他的大小随着文件结尾处变化），源窗口会载入前MAX_BLOCK_NUMBER数量的block，并放入lru的管理中。
/// @param lru lru实例
/// @param mode lru模式
/// @param srcfile 源文件实体
/// @param filesize 源文件大小
D_RT init_lru(lru_manager *lru,lru_mode mode,FILE *srcfile,size_t filesize){
    if(lru==NULL){
        printf("\nERROR: lru manager instance is NULL\n");
        return D_ERROR;
    }else if(SOURCE_WINDOW_SIZE%MAX_BLOCK_NUMBER!=/* DISABLES CODE */ (0)){
        printf("\nERROR: the max block number must be a factor of source window size\n");
        return D_ERROR;
    }else if(lru->IN_POOL_BLOCK_HASH!=NULL){
        printf("\nERROR: lru hash uninitialized to NULL\n");
        return D_ERROR;
    }
    lru->MODE=mode;
    //lru->IN_POOL_BLOCK_HASH=NULL;
    if(mode==SINGLE){
        //直接读满整个window，然后整个文件作一个block
        rewind(srcfile);
        size_t size=fread((void *)lru->source_window_pool, 1, filesize, srcfile);
        if(size!=filesize){
            printf("\nERROR: error during reading source file, mode=SINGLE\n");
            return D_ERROR;
        }
        lru->BLOCK_COUNT=1;//只有一个block
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
        //先读满整个WINDOW
        rewind(srcfile);
        size_t size=fread((void *)lru->source_window_pool, 1, SOURCE_WINDOW_SIZE, srcfile);
        if(size!=SOURCE_WINDOW_SIZE){
            printf("\nERROR: error during reading source file, mode=NORMAL\n");
            return D_ERROR;
        }
        //round up 公式：int a = (b + (c - 1)) / c;
        uint64_t init_block_size=SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER;
        lru->BLOCK_COUNT=(filesize+(init_block_size-1))/(init_block_size);//round up
        lru->BLOCK_IN_POOL=MAX_BLOCK_NUMBER;
        lru->HEAD=&lru->blk_in_pool[0];
        lru->TAIL=&lru->blk_in_pool[MAX_BLOCK_NUMBER-1];
        for(int x=0;x<MAX_BLOCK_NUMBER;x++){//设置每个block
            lru->blk_in_pool[x].BLOCK_NUMBER=x;
            lru->blk_in_pool[x].DATA=&lru->source_window_pool[x*init_block_size];
            lru->blk_in_pool[x].DATA_SIZE=init_block_size;
            if(x==0){
                lru->blk_in_pool[x].PREV=NULL;
                lru->blk_in_pool[x].NEXT=&lru->blk_in_pool[x+1];
            }else if(x==MAX_BLOCK_NUMBER-1){
                lru->blk_in_pool[x].PREV=&lru->blk_in_pool[x-1];
                lru->blk_in_pool[x].NEXT=NULL;
            }else{
                lru->blk_in_pool[x].PREV=&lru->blk_in_pool[x-1];
                lru->blk_in_pool[x].NEXT=&lru->blk_in_pool[x+1];
            }
            HASH_ADD_INT(lru->IN_POOL_BLOCK_HASH, BLOCK_NUMBER, &lru->blk_in_pool[x]);
        }
        
    }
    return D_OK;
    //[*]TODO:还要设置current window
}

void put_tail_to_head(lru_manager *lru,block *blk){
    blk->PREV->NEXT=NULL;
    lru->TAIL=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=lru->HEAD;
    lru->HEAD->PREV=blk;
    lru->HEAD=blk;
    return;
}

void put_mid_to_head(lru_manager *lru,block *blk){
    blk->PREV->NEXT=blk->NEXT;
    blk->NEXT->PREV=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=lru->HEAD;
    lru->HEAD->PREV=blk;
    lru->HEAD=blk;
    return;
}
///get_block:
/// 这个函数会自动设置源文件窗口的当前块到指定的block number，此函数会自动调解lru，如果lru中包含所需要的块，
///则把这个块放到lru最前列。如果不包含这个块，则淘汰队列最后的块，然后读取新的块放置最前列。
/// @param src SOURCE实例
/// @param blk_no 所需要的BLOCK序号
D_RT get_block(source *src,uint32_t blk_no){
    if(src==NULL){
        printf("\nERROR: source is NULL\n");
        return D_ERROR;
    }else if(src->SOURCE_FILE->FILE_INSTANCE==NULL){
        printf("\nERROR: source file is EMPTY\n");
        return D_ERROR;
    }else if(blk_no>=src->SOURCE_WINDOW->LRU_MANAGER->BLOCK_COUNT){
        printf("\nERROR: blk_no exceeds the block count\n");
        return D_ERROR;
    }
    
    
    
    lru_manager *lru=src->SOURCE_WINDOW->LRU_MANAGER;
    size_t file_size=src->SOURCE_FILE->FILE_SIZE;
    block *temp;
    
    //如果block已经是current block，直接返回
    if(src->SOURCE_WINDOW->CURRENT_BLOCK->BLOCK_NUMBER==blk_no)return D_OK;
    
    
    HASH_FIND_INT(lru->IN_POOL_BLOCK_HASH, &blk_no, temp);//查找lru中是否存在block
    if(temp==NULL){//lru中不存在所需的块
        size_t data_size;
        uint32_t data_position=(SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER)*blk_no;
        temp=lru->TAIL;
        HASH_DEL(lru->IN_POOL_BLOCK_HASH, temp);//淘汰最后面的块
        put_tail_to_head(lru,temp);//将新的块放入最前列
        temp->BLOCK_NUMBER=blk_no;
        if(blk_no==lru->BLOCK_COUNT-1 && file_size%(SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER)!=0){
            data_size=file_size%(SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER);
        }else data_size=SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER;
        memset(temp->DATA, 0, SOURCE_WINDOW_SIZE/MAX_BLOCK_NUMBER);
        fseek(src->SOURCE_FILE->FILE_INSTANCE, data_position, SEEK_SET);
        size_t size=fread((void *)temp->DATA, 1, data_size, src->SOURCE_FILE->FILE_INSTANCE);
        if(size!=data_size){
            printf("\nERROR: error during reading source file in get block, block number=%d\n",blk_no);
            return D_ERROR;
        }//读入新的数据
        //[*]TODO: 改变datasize
        temp->DATA_SIZE=data_size;
        HASH_ADD_INT(src->SOURCE_WINDOW->LRU_MANAGER->IN_POOL_BLOCK_HASH, BLOCK_NUMBER, temp);//把块重新放入hash中，此时哈希值已经更新
        src->SOURCE_WINDOW->CURRENT_BLOCK=temp;//更新current block
    }else{//队列中存在所需的块，则放到最前列
        if(temp==lru->TAIL){
            put_tail_to_head(lru,temp);
        }else{
            put_mid_to_head(lru, temp);
        }
        src->SOURCE_WINDOW->CURRENT_BLOCK=temp;//更新current block
    }
    return D_OK;
}

block *get_top_block(lru_manager *lru){
    return lru->HEAD;
}








///SOURCE



/// create_source:
///创建source实例
source *create_source(void){
    source *src=(source *)malloc(sizeof(source));
    memset(src, 0, sizeof(source));
    source_file *src_file=(source_file *)malloc(sizeof(source_file));
    memset(src_file, 0, sizeof(source_file));
    source_window *src_win=(source_window *)malloc(sizeof(source_window));
    memset(src_win, 0, sizeof(source_window));
    src_win->LRU_MANAGER=(lru_manager *)malloc(sizeof(lru_manager));
    memset(src_win->LRU_MANAGER, 0, sizeof(lru_manager));
    src_win->LRU_MANAGER->IN_POOL_BLOCK_HASH=NULL;//必须设初始化hash为NULL
    //src_win->WINDOW_SIZE=SOURCE_WINDOW_SIZE;
    src->SOURCE_FILE=src_file;
    src->SOURCE_WINDOW=src_win;
    return src;
}

D_RT set_src_file(source *src, const char *file_name){
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
    if(srcfile==NULL){
        printf("\nERROR: source file doesn't exist\n");
        return D_ERROR;
    }
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
        if(init_lru(lru, SINGLE, src->SOURCE_FILE->FILE_INSTANCE, srcsize)!=D_OK)exit(0);
        srcwin->CURRENT_BLOCK=get_top_block(lru);
        srcwin->WINDOW_SIZE=srcsize;
        //srcwin->BLOCK_COUNT=1;
        //HASH_ADD_INT( lru->IN_POOL_BLOCK_HASH, id, s );
    }else{
        if(init_lru(lru, NORMAL, src->SOURCE_FILE->FILE_INSTANCE, srcsize)!=D_OK)exit(0);
        srcwin->CURRENT_BLOCK=get_top_block(lru);
        srcwin->WINDOW_SIZE=SOURCE_WINDOW_SIZE;
    }
    return D_OK;
}

void add_position(source_hash *h,int x){
    source_position *p=(source_position *)malloc(sizeof(source_position));
    p->position=x;
    p->next=h->head;
    h->head=p;
    h->cnt++;
    return;
}
//source_hash *SOURCE_HASH_LIST;
/// global_source_hash:
/// 全局的源文件哈希。
/// @param src SOURCE实例
D_RT global_source_hash(source *src){
    char buffer[CRC_LEN];
    uint16_t crc;
    size_t filesize=src->SOURCE_FILE->FILE_SIZE;
    crc16speed_init();
    for(int x=0;x<filesize-CRC_LEN;x++){
        if (get_n_char_at(src, x, buffer)!=D_OK)exit(0);
        crc=crc16speed(0, buffer, CRC_LEN);
        source_hash *s;
        HASH_FIND(hh, src->SOURCE_HASH, &crc, 2, s);
        if (s==NULL) {
            s = (source_hash *)malloc(sizeof(source_hash));
            s->crc=crc;
            s->cnt=0;
            s->head=NULL;
            add_position(s,x);
            HASH_ADD(hh, src->SOURCE_HASH, crc, 2, s);
        }else{
            add_position(s, x);
        }
    }
    return D_OK;
}


D_RT get_n_char_at(source *src,uint32_t position, char buffer[]){
    lru_manager *lru=src->SOURCE_WINDOW->LRU_MANAGER;
    uint64_t init_blk_size=src->SOURCE_WINDOW->WINDOW_SIZE/
    src->SOURCE_WINDOW->LRU_MANAGER->BLOCK_IN_POOL;
    if(position+CRC_LEN-1>=src->SOURCE_FILE->FILE_SIZE){
        printf("\nERROR: exceeds file size\n");
        return D_ERROR;
    }else if(CRC_LEN>init_blk_size){
        printf("\nERROR: cannot read morethan one block\n");
        return D_ERROR;
    }
    //需要分辨lru的mode再决定
    if(lru->MODE==SINGLE){
        int x,y;
        for(x=position, y=0;y<CRC_LEN;x++,y++){
            buffer[y]=src->SOURCE_WINDOW->CURRENT_BLOCK->DATA[x];
        }
    }else{
        uint32_t blk_no=position/(init_blk_size);
        uint32_t local_position=position%(init_blk_size);
        if (get_block(src, blk_no)!=D_OK)exit(0);
        if (local_position+CRC_LEN-1>=init_blk_size){
            uint8_t compensate=(uint8_t)(local_position+CRC_LEN-init_blk_size);
            int x,y;
            for(x=local_position, y=0;x<init_blk_size;x++,y++){
                buffer[y]=src->SOURCE_WINDOW->CURRENT_BLOCK->DATA[x];
            }
            if (get_block(src, blk_no+1)!=D_OK)exit(0);
            for(x=0;x<compensate;x++,y++){
                buffer[y]=src->SOURCE_WINDOW->CURRENT_BLOCK->DATA[x];
            }
        }else{
            int x,y;
            for(x=local_position, y=0;y<CRC_LEN;x++,y++){
                buffer[y]=src->SOURCE_WINDOW->CURRENT_BLOCK->DATA[x];
            }
        }
    }
    return D_OK;
}

char get_char_at(source *src,uint32_t position){
    //lru_manager *lru=src->SOURCE_WINDOW->LRU_MANAGER;
    uint64_t init_blk_size=src->SOURCE_WINDOW->WINDOW_SIZE/
    src->SOURCE_WINDOW->LRU_MANAGER->BLOCK_IN_POOL;
    if(position>=src->SOURCE_FILE->FILE_SIZE){
        printf("\nERROR: exceeds file size\n");
        exit(0);
    }
    uint32_t blk_no=position/(init_blk_size);
    uint32_t local_position=position%(init_blk_size);
    if (get_block(src, blk_no)!=D_OK)exit(0);
    return src->SOURCE_WINDOW->CURRENT_BLOCK->DATA[local_position];
    
}

D_RT clean_source(source *src){
    source_hash *current, *tmp;

    fclose(src->SOURCE_FILE->FILE_INSTANCE);
    free(src->SOURCE_FILE);
    
    HASH_ITER(hh, src->SOURCE_HASH, current, tmp) {
        HASH_DEL(src->SOURCE_HASH,current);
        source_position *posi=current->head;
        source_position *posi_next;
        while(posi!=NULL){
            posi_next=posi->next;
            free(posi);
            posi=posi_next;
        }
        free(current);
    }
    HASH_CLEAR(hh,src->SOURCE_WINDOW->LRU_MANAGER->IN_POOL_BLOCK_HASH);
    free(src->SOURCE_WINDOW->LRU_MANAGER);
    free(src->SOURCE_WINDOW);
    free(src);
    return D_OK;
}

void source_md5(source *src,unsigned char md5[16]){
	unsigned char check_buff[2048];
	uint32_t calculated=0;
	FILE *src_file=src->SOURCE_FILE->FILE_INSTANCE;
	size_t src_size=src->SOURCE_FILE->FILE_SIZE;
	MD5_CTX md5_checker;
	MD5Init(&md5_checker);
	rewind(src_file);
	while(calculated<src_size){
		uint32_t copy_size=delta_min((uint32_t)src_size-calculated,2048);
        fread((void *)check_buff, 1, copy_size, src_file);
        MD5Update(&md5_checker,check_buff,copy_size);
        calculated+=copy_size;
	}
	MD5Final(md5,&md5_checker);
	rewind(src_file);
	return;

} 

