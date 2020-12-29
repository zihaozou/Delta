//
//  VCDIFF.h
//  Delta
//
//  Created by 邹子豪 on 12/28/20.
//

#ifndef VCDIFF_h
#define VCDIFF_h
#include "type_def.h"

static const int DeltaDefaultCodeTable[6][256] =
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


void count_int_len_test(void);
#endif /* VCDIFF_h */
