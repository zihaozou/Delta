//
//  main.c
//  Delta
//
//  Created by 邹子豪 on 12/18/20.
//
//#include "SOURCE.h"
//#include "INSTRUCTION.h"
//#include "TARGET.h"
//#include "STREAM.h"
//#include "VCDIFF.h"
#include "DECODER.h"
#include "ENCODER.h"
//#include "argparse.h"
//static source *Source;
const char *helpstring="\n命令格式：\n帮助： -h\n编码： ./delta -e 源文件名 目标文件名 DELTA文件名\n解码： ./delta -d DELTA文件名 源文件名 升级后的文件名\n";
int main(int argc, const char * argv[]) {
    printf("%s", helpstring);
    if(argc!=5){
        printf("\nERROR invalid argument\n");
        return 0;
    }
    if(!strcmp(argv[1], "-e")){
        ENCODER(argv[2],argv[3],argv[4]);
    }else if(!strcmp(argv[1], "-d")){
        DECODER(argv[2], argv[3], argv[4]);
    }else if(!strcmp(argv[1], "-h")){
        printf("%s", helpstring);
    }else{
        printf("\nERROR invalid argument\n");
        return 0;
    }
    //ENCODER("test/old.bin","test/new.bin","test/delta_file");
    printf("\nDONE\n");
    return 0;
}
