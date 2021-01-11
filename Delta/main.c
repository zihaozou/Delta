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
const char *helpstring="\n编码模式：\n1，在新空间上独立解码出目标文件\n2，直接覆盖源文件解码\n3，源文件后退式覆盖\n命令格式：\n帮助： -h\n编码： ./delta -e 源文件名 目标文件名 DELTA文件名 编码模式（-1/-2/-3，如果不填写编码模式，则默认为1模式） 窗口大小\n解码： ./delta -d DELTA文件名 源文件名 升级后的文件名 窗口大小\n";
int main(int argc, const char * argv[]) {
    printf("%s", helpstring);
    if(argc<6 || argc>7){
        printf("\nERROR invalid argument\n");
        return 0;
    }
    if(!strcmp(argv[1], "-e")){
        if(argc==6){
            ENCODER(argv[2],argv[3],argv[4],1,atoi(argv[6]));
        }else if(argc==7){
            if(!strcmp("-1", argv[5])){
                ENCODER(argv[2],argv[3],argv[4],1,atoi(argv[6]));
            }else if (!strcmp("-2", argv[5])){
                ENCODER(argv[2],argv[3],argv[4],2,atoi(argv[6]));
            }else if(!strcmp("-3", argv[5])){
                ENCODER(argv[2],argv[3],argv[4],3,atoi(argv[6]));
            }
        }else{
            printf("\nERROR invalid argument\n");
            return 0;
        }
    }else if(!strcmp(argv[1], "-d")){
        if(argc!=6){
            printf("\nERROR invalid argument\n");
            return 0;
        }
        DECODER(argv[2], argv[3], argv[4],atoi(argv[5]));
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
