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
//  ENCODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#include "ENCODER.h"

uint32_t mode3_backoff(stream *Stream);
void create_alternate_src(source *src);
void ENCODER(const char *source_name,const char *target_name,const char *delta_name,int mode,int page_size){
    size_t srcsize,delsize;
    FILE *output;
    uint32_t add_size=0;
    stream *Stream=create_stream();
    target *Target=create_target(page_size);
    source *Source=create_source();
    Stream->ENCODE_MODE=mode;
    if(set_src_file(Source, source_name)==D_ERROR || set_tgt_file(Target, target_name)==D_ERROR)
        return;
    srcsize=Source->SOURCE_FILE->FILE_SIZE;
    if(mode!=1)
        create_alternate_src(Source);
    init_window(Source);
    global_source_hash(Source);
    add_target(Stream, Target);
    add_source(Stream, Source);
    output=fopen(delta_name, "wb+");
    header_packer(output, Stream);
    if (mode==3){
        add_size=mode3_backoff(Stream);
        mode3_add_size_packer(output,Stream,add_size);
    }
    while(set_window(Target)==D_OK){
        init_encode(Stream);
        match(Stream);
        add_size=ADD_complement(Stream->TARGET->TARGET_WINDOW->INSTRUCTION, Stream->TARGET->TARGET_WINDOW);
        window_packer(output, Stream);
        if(mode!=1)rearrange_source_file(Stream, add_size);
    }
	if(delta_md5(output)==D_ERROR){
		fclose(output);
		remove(delta_name);
		return;
	}
    delete_inst_list(Target->TARGET_WINDOW->INSTRUCTION);
    fseek(output, 0, SEEK_END);
    delsize=ftell(output);
    printf("\nSUCCEED\n");
    printf("总结：\n源文件大小: %zuBytes\n目标文件大小: %zuBytes\n差分文件大小: %zuBytes\n",srcsize,Target->TARGET_FILE->FILE_SIZE,delsize);
    if(mode==3){
        printf("模式3单片机预留空间需大于：%zuBytes\n",Stream->SOURCE->SOURCE_FILE->FILE_SIZE);
    }
    fclose(output);
    clean_source(Source);
    if(mode!=1){
        remove("temp.bin");
    }
    clean_target(Target);
    
}

void create_alternate_src(source *src){
    FILE *temp=fopen("temp.bin", "wb+");
    byte *tempbuffer=(byte *)malloc(sizeof(byte)*src->SOURCE_FILE->FILE_SIZE);
    fread(tempbuffer, 1, src->SOURCE_FILE->FILE_SIZE, src->SOURCE_FILE->FILE_INSTANCE);
    fwrite(tempbuffer, src->SOURCE_FILE->FILE_SIZE, 1, temp);
    fclose(src->SOURCE_FILE->FILE_INSTANCE);
    src->SOURCE_FILE->FILE_INSTANCE=temp;
    rewind(temp);
    free(tempbuffer);
}

uint32_t mode3_backoff(stream *Stream){
    uint32_t total_bf_size=0;
    while(set_window(Stream->TARGET)==D_OK){
        init_encode(Stream);
        match(Stream);
        total_bf_size+=ADD_complement(Stream->TARGET->TARGET_WINDOW->INSTRUCTION, Stream->TARGET->TARGET_WINDOW);
    }
    //delete_inst_list(Stream->TARGET->TARGET_WINDOW->INSTRUCTION);
    rewind(Stream->TARGET->TARGET_FILE->FILE_INSTANCE);
    rewind(Stream->SOURCE->SOURCE_FILE->FILE_INSTANCE);
    rearrange_source_file(Stream,total_bf_size);
    return total_bf_size;
}




