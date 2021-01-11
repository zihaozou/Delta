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
//  DECODER.h
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//

#ifndef DECODER_h
#define DECODER_h
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "flash.h"
#define SOFTWARE_VERSION 0x01//软件版本，在收到的delta文件中，也有记录一个版本信息，只有当两个版本符合的话，才可以进行匹配。
//#include "utlist.h"
#define delta_min(a,b)      ((a)<(b))?(a):(b)//最小值比较
#define delta_max(a,b)      ((a)>(b))?(a):(b)//最大值比较
#define s_near 4//near缓存区大小
#define s_same 3//same缓存区大小
#if s_near<=0
#define s_near 4
#endif
#if s_same<=0
#define s_same 3
#endif
#define VCD_SELF 0//SELF寻址模式
#define VCD_HERE 1//HERE寻址模式
#define DEFAULT_SOURCE_DATA_POSITION 0x8008000//源文件被存放的起始位置，注意：源文件必须被存放在一个flash page的起始位置
#define DEFAULT_DELTA_DATA_POSITION 0x8004000//delta文件存放的起始位置
#define DEFAULT_UPDATED_DATA_POSITION 0x8020000//如果需要将解码出的目标文件放在一个新的地址空间上，则需要在这里定义其位置
#define DEBUG_DELTA_SIZE 15469//调试用delta文件大小
#define DEBUG_SOURCE_SIZE 95072//调试用源文件大小
#define DECODE_SOURCE_SIZE 1024//解码器LRU缓存区大小
#define DECODE_BLOCK_NUMBER 4//LRU的BLOCK数量，单个BLOCK缓存区大小为DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER
typedef uint32_t dsize;//大小变量类型
typedef uint32_t dposition;//位置变量类型
typedef unsigned char byte;//数据变量类型
typedef enum delta_return{//返回值定义
    D_OK,
    D_ERROR,
    D_FULL,
    D_EMPTY,
    D_NONE,
    D_EXIST,
    D_NONEXIST
}D_RT;
typedef struct _decode_block{//LRU的BLOCK定义
    int BLOCK_NUMBER;
    byte *DATA;//存放数据的地址
    size_t DATA_SIZE;//数据大小
    struct _decode_block *PREV;
    struct _decode_block *NEXT;
}decode_block;
typedef struct _dfile{//文件类型定义
	uint32_t FileBeginAddr;//文件的起始地址
	uint32_t FileOffset;//当前处理文件的偏移指针
}dfile;
typedef struct _delta{//这个结构体存有解码所需的所有信息，会在各个函数中传递
    dfile DELTA_FILE;//delta文件
	dsize DELTA_SIZE;//delta文件大小,这里需要在接受delta文件时被赋值
	
    dfile UPDATED_FILE;//目标文件
	dsize UPDATED_SIZE;//目标文件大小，
	
	
    dfile SOURCE_FILE;//源文件
	dsize SOURCE_SIZE;//源文件大小，这个是必须的，我们的IAP需要某种方式来记录源文件大小，并在init_delta函数中赋值给这个变量
	
	byte DECODE_MODE;
    byte UPDATED_BUFFER[FLASH_PAGE_SIZE];//解码数据缓存区
    byte MOVE_DATA_BUFFER[FLASH_PAGE_SIZE];//移动flash中源文件数据的缓存区
    byte SOURCE_BUFFER[DECODE_SOURCE_SIZE];//源文件数据数据                    TODO：这个缓存区可以与MOVE_DATA_BUFFER合并，来减少内存的使用
    decode_block DECODE_BLOCK_LIST[DECODE_BLOCK_NUMBER];//Lru BLOCK列表
    decode_block *CURRENT_BLOCK;//当前解码BLOCK
    decode_block *HEAD;
    decode_block *TAIL;
    uint32_t BLOCK_COUNT;
    uint8_t BLOCK_INPOOL;
	
    //下面为vcdiff所需数据
    byte HDR_INDICATOR;//
    //暂时没有二次压缩和自定义表
    //window
    byte WIN_INDICATOR;
    dsize SOURCE_SEGMENT_LENGTH;
    dposition SOURCE_SEGMENT_POSITION;
    //delta encode
    dsize LENGTH_DELTA_ENCODE;
    dsize LENGTH_TARGET_WIN;
    byte DELTA_INDICATOR;
    dsize LEN_DATA;
    dsize LEN_INST;
    dsize LEN_ADDR;
	
	
    dposition CURRENT_WIN_POSI;
    dposition NEXT_WIN_POSI;
    dposition CURRENT_DATA_POSI;
    dposition CURRENT_INST_POSI;
    dposition CURRENT_ADDR_POSI;
}delta;
typedef enum _inst_type{
    COPY,
    ADD,
    RUN,
    NOOP,
    SCOPY//self copy
}inst_type;//delta指令类型
typedef struct _addr_cache{
    uint32_t near[s_near];
    uint8_t next_slot;
    uint32_t same[s_same*256];
} addr_cache;//地址缓存
static const uint8_t DeltaDefaultCodeTable[6][256] =
    // 0:inst1
    { { RUN,  // opcode 0
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 1-18
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 19-34
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 35-50
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 51-66
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 67-82
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 83-98
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 99-114
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 115-130
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 131-146
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 147-162
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 163-174
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 175-186
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 187-198
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 199-210
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 211-222
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD,  // opcodes 223-234
        ADD, ADD, ADD, ADD,  // opcodes 235-238
        ADD, ADD, ADD, ADD,  // opcodes 239-242
        ADD, ADD, ADD, ADD,  // opcodes 243-246
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY },  // opcodes 247-255
    // 1:inst2
      { NOOP,  // opcode 0
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 1-18
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 19-34
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 35-50
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 51-66
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 67-82
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 83-98
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 99-114
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 115-130
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 131-146
        NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,  // opcodes 147-162
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 163-174
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 175-186
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 187-198
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 199-210
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 211-222
        COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY, COPY,  // opcodes 223-234
        COPY, COPY, COPY, COPY,  // opcodes 235-238
        COPY, COPY, COPY, COPY,  // opcodes 239-242
        COPY, COPY, COPY, COPY,  // opcodes 243-246
        ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD, ADD },  // opcodes 247-255
    // 2:size1
      { 0,  // opcode 0
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,  // 1-18
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 19-34
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 35-50
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 51-66
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 67-82
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 83-98
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 99-114
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 115-130
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 131-146
        0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,  // 147-162
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 163-174
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 175-186
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 187-198
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 199-210
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 211-222
        1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,  // opcodes 223-234
        1, 2, 3, 4,  // opcodes 235-238
        1, 2, 3, 4,  // opcodes 239-242
        1, 2, 3, 4,  // opcodes 243-246
        4, 4, 4, 4, 4, 4, 4, 4, 4 },  // opcodes 247-255
    // 3:size2
      { 0,  // opcode 0
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 1-18
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 19-34
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 35-50
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 51-66
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 67-82
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 83-98
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 99-114
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 115-130
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 131-146
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 147-162
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 163-174
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 175-186
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 187-198
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 199-210
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 211-222
        4, 5, 6, 4, 5, 6, 4, 5, 6, 4, 5, 6,  // opcodes 223-234
        4, 4, 4, 4,  // opcodes 235-238
        4, 4, 4, 4,  // opcodes 239-242
        4, 4, 4, 4,  // opcodes 243-246
        1, 1, 1, 1, 1, 1, 1, 1, 1 },  // opcodes 247-255
    // 4:mode1
      { 0,  // opcode 0
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 1-18
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 19-34
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // opcodes 35-50
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // opcodes 51-66
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // opcodes 67-82
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  // opcodes 83-98
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  // opcodes 99-114
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,  // opcodes 115-130
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  // opcodes 131-146
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  // opcodes 147-162
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 163-174
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 175-186
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 187-198
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 199-210
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 211-222
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 223-234
        0, 0, 0, 0,  // opcodes 235-238
        0, 0, 0, 0,  // opcodes 239-242
        0, 0, 0, 0,  // opcodes 243-246
        0, 1, 2, 3, 4, 5, 6, 7, 8 },  // opcodes 247-255
    // 5:mode2
      { 0,  // opcode 0
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 1-18
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 19-34
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 35-50
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 51-66
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 67-82
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 83-98
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 99-114
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 115-130
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 131-146
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 147-162
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // opcodes 163-174
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // opcodes 175-186
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // opcodes 187-198
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // opcodes 199-210
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  // opcodes 211-222
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  // opcodes 223-234
        6, 6, 6, 6,  // opcodes 235-238
        7, 7, 7, 7,  // opcodes 239-242
        8, 8, 8, 8,  // opcodes 243-246
        0, 0, 0, 0, 0, 0, 0, 0, 0 } };  // opcodes 247-255
void DECODER(void);
#endif /* DECODER_h */
