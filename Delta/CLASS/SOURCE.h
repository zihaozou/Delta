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
//  SOURCE.h
//  Delta
//
//  Created by 邹子豪 on 12/21/20.
//

#ifndef SOURCE_h
#define SOURCE_h

#include <stdio.h>
#include "type_def.h"
#include "md5.h"
/*
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
 
 */


source *create_source(void);
D_RT set_src_file(source *src, const char *file_name);
//D_RT set_hash_table()
D_RT init_window(source *src);
D_RT global_source_hash(source *src);
char get_char_at(source *src,uint32_t position);
D_RT get_block(source *src,uint32_t blk_no);
D_RT get_n_char_at(source *src,uint32_t position, char buffer[]);
D_RT clean_source(source *src);
void add_position(source_hash *h,int x);
void source_md5(source *src,unsigned char md5[16]);
#endif /* SOURCE_h */
