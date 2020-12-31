//
//  ENCODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#include "ENCODER.h"



void ENCODER(const char *source_name,const char *target_name,const char *delta_name){
    FILE *output=fopen(delta_name, "wb+");
    stream *Stream=create_stream();
    target *Target=create_target();
    source *Source=create_source();
    //instruction *Instruction=create_instruction();
    set_src_file(Source, source_name);
    init_window(Source);
    global_source_hash(Source);
    set_tgt_file(Target, target_name);
    add_target(Stream, Target);
    add_source(Stream, Source);
    header_packer(output, Stream);
    
    while(set_window(Target)==D_OK){
        init_encode(Stream);
        match(Stream);
        ADD_complement(Stream->TARGET->TARGET_WINDOW->INSTRUCTION, Stream->TARGET->TARGET_WINDOW);
        window_packer(output, Stream);
    }
    fclose(output);
    delete_inst_list(Target->TARGET_WINDOW->INSTRUCTION);
    clean_source(Source);
    clean_target(Target);
}
