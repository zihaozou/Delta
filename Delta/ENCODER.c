//
//  ENCODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#include "ENCODER.h"



void ENCODER(const char *source_name,const char *target_name,const char *delta_name,int mode,int page_size){
    FILE *output=fopen(delta_name, "wb+");
    uint32_t add_size;
    FILE *temp;
    stream *Stream=create_stream();
    target *Target=create_target(page_size);
    source *Source=create_source();
    Stream->ENCODE_MODE=mode;
    set_src_file(Source, source_name);
    if(mode!=1){
        temp=fopen("temp.bin", "wb+");
        byte *tempbuffer=(byte *)malloc(sizeof(byte)*Source->SOURCE_FILE->FILE_SIZE);
        fread(tempbuffer, 1, Source->SOURCE_FILE->FILE_SIZE, Source->SOURCE_FILE->FILE_INSTANCE);
        fwrite(tempbuffer, Source->SOURCE_FILE->FILE_SIZE, 1, temp);
        fclose(Source->SOURCE_FILE->FILE_INSTANCE);
        Source->SOURCE_FILE->FILE_INSTANCE=temp;
        rewind(temp);
        free(tempbuffer);
    }
    init_window(Source);
    global_source_hash(Source);
    set_tgt_file(Target, target_name);
    add_target(Stream, Target);
    add_source(Stream, Source);
    header_packer(output, Stream);
    
    while(set_window(Target)==D_OK){
        init_encode(Stream);
        match(Stream);
        add_size=ADD_complement(Stream->TARGET->TARGET_WINDOW->INSTRUCTION, Stream->TARGET->TARGET_WINDOW);
        window_packer(output, Stream);
        if(mode!=1)rearrange_source_file(Stream, add_size);
    }
    fclose(output);
    delete_inst_list(Target->TARGET_WINDOW->INSTRUCTION);
    clean_source(Source);
    if(mode!=1){
        remove("temp.bin");
    }
    clean_target(Target);
}
