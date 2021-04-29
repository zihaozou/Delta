//
//  DECODER.c
//  Delta
//
//  Created by 邹子豪 on 12/30/20.
//
#include "DECODER.h"
//全局变量
static addr_cache cache;
static delta Del;
//解码相关函数
void init_delta(delta *del);
void set_win_header(delta *del);
D_RT verify_file(delta *del);
D_RT decode_window(delta *del);
void set_first_win(delta *del);
void set_HDR(delta *del);
D_RT finalizer(delta *del);
/*文件操作函数*/
D_RT dfile_open(dfile *Open_File,uint32_t Addr);
byte read_byte(dfile *File);
uint32_t read_integer(dfile *f);
uint64_t read_integer_at(dfile *f,dposition location);
void read_bytes_at(dfile *f,dsize s,dposition p,byte *b);
byte read_byte_at(dfile *f,dposition location);

uint32_t write_data(dfile *f,byte *buffToSave, uint32_t len);
uint32_t write_page_at(uint32_t page_addr, byte *page_content, uint32_t len);

/*地址缓存相关函数*/
D_RT cache_init(addr_cache *cc);
D_RT cache_update(addr_cache *cc,uint64_t addr);
/*LRU相关函数*/
D_RT init_source_lru(delta *del);
void decode_put_tail_to_head(delta *del,decode_block *blk);
void decode_put_mid_to_head(delta *del,decode_block *blk);
decode_block *find_block_in_pool(delta *del,uint32_t blk_no);
D_RT decode_get_block(delta *del,uint32_t blk_no);
D_RT copy_data(delta *del,dposition decode_posi, dposition addr,dsize siz);
/*****************************************************************************
 * 函 数 名  : DECODER
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.01.09
 * 函数功能  : 开始解码流程
 * 输入参数  : 无
 * 输出参数  : 无
<<<<<<< HEAD
 * 返 回 值  : 无
=======
 * 返 回 值  : D_OK: 解码成功
				D_ERROR：解码失败
>>>>>>> board_code
 * 调用关系  :
 * 其    它  :
	解码主要流程：
	1,初始化delta文件，源文件和目标文件，设置他们在flash上的起始地址和他们的大小
	2,检验delta文件是否正确，检验文件的magic bytes和版本信息
	3,设置HDR，如需二次解压缩则启动二次解压缩流程
	4，设置第一个目标窗口的起始位置
	5，启动窗口解码，此处进入一个循环，一直解码下一窗口，直到读到delta文件末尾
*****************************************************************************/

D_RT DECODER(){
    //delta *Del=create_delta(delta_name,updated_name,source_name);
	uint32_t x;
	dposition dest_page;
	dsize copy_size;
	dposition left_mark;
	dsize extended_size;
	dsize add_size;
    init_delta(&Del);
    if(verify_file(&Del)==D_ERROR)return D_ERROR;
    
    set_HDR(&Del);
	if(Del.DECODE_MODE==3){//mode3：将整个源文件后退
		add_size=read_integer(&Del.DELTA_FILE);
		left_mark=Del.SOURCE_SIZE;
		extended_size=Del.SOURCE_SIZE+add_size;
		while(left_mark-Del.UPDATED_FILE.FileOffset){
			copy_size=(extended_size%DEFAULT_UPDATED_WIN_SIZE==0)?delta_min(left_mark-Del.UPDATED_FILE.FileOffset,
			DEFAULT_UPDATED_WIN_SIZE):extended_size%DEFAULT_UPDATED_WIN_SIZE;
			memset(Del.MOVE_DATA_BUFFER,0,DEFAULT_UPDATED_WIN_SIZE);
			read_flash_at ( Del.SOURCE_FILE.FileBeginAddr+left_mark-copy_size, 
			&Del.MOVE_DATA_BUFFER[(left_mark==copy_size)?DEFAULT_UPDATED_WIN_SIZE-copy_size:0], copy_size );
			dest_page=Del.SOURCE_FILE.FileBeginAddr+((extended_size+(DEFAULT_UPDATED_WIN_SIZE-1))
			/DEFAULT_UPDATED_WIN_SIZE-1)*DEFAULT_UPDATED_WIN_SIZE;
			erase_page_at ( dest_page, DEFAULT_UPDATED_WIN_SIZE );
			write_page_at ( dest_page, Del.MOVE_DATA_BUFFER, (left_mark==copy_size)?DEFAULT_UPDATED_WIN_SIZE:copy_size );
			left_mark-=copy_size;
			extended_size-=copy_size;
		}
		memset(Del.MOVE_DATA_BUFFER,0,DEFAULT_UPDATED_WIN_SIZE);
		for(x=Del.SOURCE_FILE.FileBeginAddr;x<dest_page;x+=DEFAULT_UPDATED_WIN_SIZE){
			erase_page_at ( x, DEFAULT_UPDATED_WIN_SIZE );
			write_page_at(x,Del.MOVE_DATA_BUFFER,DEFAULT_UPDATED_WIN_SIZE);
		}
		Del.SOURCE_SIZE+=add_size;
	}
	set_first_win(&Del);
    while(Del.NEXT_WIN_POSI<Del.DELTA_SIZE/*这里使用的delta size必须在init_delta中设置好*/){
        if(decode_window(&Del)==D_ERROR)return D_ERROR;
    }
	finalizer(&Del);
	return D_OK;

}
/*****************************************************************************
 * 函 数 名  : dfile_open
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 打开指定的文件，会设置文件的起始地址，然后归零偏移指针
 * 输入参数  : dfile *Open_File：要打开的文件
			   uint32_t Addr：   文件的起始地址
 * 输出参数  : 无
 * 返 回 值  : D_OK：打开成功
			   D_ERROR：打开失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT dfile_open(dfile *Open_File,uint32_t Addr){
	if(Addr<FLASH_ADDR_BASE ||Addr>FLASH_ADDR_BASE+FLASH_SIZE)return D_ERROR;
	Open_File->FileBeginAddr=Addr;
	Open_File->FileOffset=0;
	return D_OK;
}
/*****************************************************************************
 * 函 数 名  : read_byte
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从指定文件读取一个byte
 * 输入参数  : dfile *File：文件实例
 * 输出参数  : 无
 * 返 回 值  : byte temp：读到的byte数据
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
byte read_byte(dfile *File){
	byte temp;
	read_flash_at ( File->FileBeginAddr+File->FileOffset, &temp, 1 );
	File->FileOffset++;
    return temp;
}
/*****************************************************************************
 * 函 数 名  : read_integer
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从指定的文件中读取一个integer
 * 输入参数  : dfile *f：文件实例
 * 输出参数  :
 * 返 回 值  : uint32_t integer： 读到的integer
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
uint32_t read_integer(dfile *f){
    uint32_t integer=0;
    byte temp=0;
    do {
        integer<<=7;
        temp=read_byte(f);
        integer|=(temp & (~0x80));
    } while (temp & 0x80);
    return integer;
}
/*****************************************************************************
 * 函 数 名  : read_integer_at
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从指定文件在指定位置读取一个integer
 * 输入参数  : dfile *f：             文件实例
			   dposition location：   指定位置
 * 输出参数  :
 * 返 回 值  : uint32_t：读取到的integer
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
uint64_t read_integer_at(dfile *f,dposition location){
    f->FileOffset=location;
    return read_integer(f);
}
/*****************************************************************************
 * 函 数 名  : read_bytes_at
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从指定文件在指定位置读取多个byte
 * 输入参数  : dfile *f：   文件实例
			   dsize s ：   读取大小
			   dposition p：读取位置
			   byte *b：    缓存区
 * 输出参数  :
 * 返 回 值  :
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void read_bytes_at(dfile *f,dsize s,dposition p,byte *b){
    f->FileOffset=p;

    read_flash_at ( f->FileBeginAddr+f->FileOffset, b, s );

	f->FileOffset+=s;
    return;
}
/*****************************************************************************
 * 函 数 名  : read_byte_at
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从指定文件在指定位置读取一个byte
 * 输入参数  : dfile *f：            文件实例
			   dposition location：  指定位置
 * 输出参数  :
 * 返 回 值  : byte：读到的byte
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
byte read_byte_at(dfile *f,dposition location){
    f->FileOffset=location;
    return read_byte(f);
}
/*****************************************************************************
 * 函 数 名  : write_data
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 向指定文件写入数据
 * 输入参数  : dfile *f：         文件实例
			   u8 *buffToSave：   需要写入的缓存区
			   u32 len：		  需要写入的大小
 * 输出参数  :
 * 返 回 值  : 写入的数据大小，为0则写入失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
uint32_t write_data(dfile *f,byte *buffToSave, uint32_t len){
	uint32_t temp=write_flash_at ( f->FileBeginAddr+f->FileOffset, buffToSave,len );
	f->FileOffset+=temp;
	return temp;
}

/*****************************************************************************
 * 函 数 名  : write_page_at
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.4.27
 * 函数功能  : 写入整个flash page
 * 输入参数  : uint32_t page_addr：page所在的起始地址
			   byte *page_content：   需要写入的缓存区
			   uint32_t len：		  需要写入的大小
 * 输出参数  :
 * 返 回 值  : 写入的数据大小，为0则写入失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
uint32_t write_page_at(uint32_t page_addr, byte *page_content, uint32_t len){
	return write_flash_at(page_addr, page_content,len);
}

/*****************************************************************************
 * 函 数 名  : cache_init
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 初始化地址缓存
 * 输入参数  : addr_cache *cc：地址缓存实例
 * 输出参数  :
 * 返 回 值  :
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT cache_init(addr_cache *cc){
    int i;
    cc->next_slot=0;
    for(i=0;i<s_near;i++)cc->near[i]=0;
    for (i=0; i<s_same*256; i++)cc->same[i]=0;
    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : cache_update
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 更新缓存区
 * 输入参数  : addr_cache *cc：缓存区实例
			   uint64_t addr：要存储的地址
 * 输出参数  :
 * 返 回 值  :
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT cache_update(addr_cache *cc,uint64_t addr){
    cc->near[cc->next_slot]=addr;
    cc->next_slot=(cc->next_slot+1)%s_near;
    cc->same[addr%(s_same*256)]=addr;
    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : init_source_lru
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 初始化lru，根据需要的源文件片段，填充lru block缓存区
 * 输入参数  : delta *del：delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT init_source_lru(delta *del){
    if(del->SOURCE_SEGMENT_LENGTH<=DECODE_SOURCE_SIZE){
        //del->SOURCE_FILE.FileOffset=del->SOURCE_SEGMENT_POSITION;

		read_bytes_at(&del->SOURCE_FILE,del->SOURCE_SEGMENT_LENGTH,del->SOURCE_SEGMENT_POSITION,del->MOVE_DATA_BUFFER);

        del->BLOCK_COUNT=1;
        del->BLOCK_INPOOL=1;
        del->HEAD=&del->DECODE_BLOCK_LIST[0];
        del->TAIL=&del->DECODE_BLOCK_LIST[0];
        del->DECODE_BLOCK_LIST[0].BLOCK_NUMBER=0;

        del->DECODE_BLOCK_LIST[0].DATA=del->MOVE_DATA_BUFFER;

        del->DECODE_BLOCK_LIST[0].DATA_SIZE=del->SOURCE_SEGMENT_LENGTH;
        del->DECODE_BLOCK_LIST[0].NEXT=NULL;
        del->DECODE_BLOCK_LIST[0].PREV=NULL;
        //del->SOURCE_WIN_SIZE=del->SOURCE_SEGMENT_LENGTH;
    }else{

        read_bytes_at(&del->SOURCE_FILE,DECODE_SOURCE_SIZE,del->SOURCE_SEGMENT_POSITION,del->MOVE_DATA_BUFFER);

        uint64_t init_block_size=DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER;
        //a = (b + (c - 1)) / c
        del->BLOCK_COUNT=(uint32_t)(del->SOURCE_SEGMENT_LENGTH+(init_block_size-1))/init_block_size;
        del->BLOCK_INPOOL=DECODE_BLOCK_NUMBER;
        del->HEAD=&del->DECODE_BLOCK_LIST[0];
        del->TAIL=&del->DECODE_BLOCK_LIST[DECODE_BLOCK_NUMBER-1];
        for(int x=0;x<DECODE_BLOCK_NUMBER;x++){//设置每个block
            del->DECODE_BLOCK_LIST[x].BLOCK_NUMBER=x;

            del->DECODE_BLOCK_LIST[x].DATA=&del->MOVE_DATA_BUFFER[x*init_block_size];

            del->DECODE_BLOCK_LIST[x].DATA_SIZE=init_block_size;
            if(x==0){
                del->DECODE_BLOCK_LIST[x].PREV=NULL;
                del->DECODE_BLOCK_LIST[x].NEXT=&del->DECODE_BLOCK_LIST[x+1];
            }else if(x==DECODE_BLOCK_NUMBER-1){
                del->DECODE_BLOCK_LIST[x].PREV=&del->DECODE_BLOCK_LIST[x-1];
                del->DECODE_BLOCK_LIST[x].NEXT=NULL;
            }else{
                del->DECODE_BLOCK_LIST[x].PREV=&del->DECODE_BLOCK_LIST[x-1];
                del->DECODE_BLOCK_LIST[x].NEXT=&del->DECODE_BLOCK_LIST[x+1];
            }
            //HASH_ADD_INT(lru->IN_POOL_BLOCK_HASH, BLOCK_NUMBER, &lru->blk_in_pool[x]);
        }
    }
    del->CURRENT_BLOCK=&del->DECODE_BLOCK_LIST[0];

    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : decode_put_tail_to_head
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 把block链表中的最后一位放到第一位
 * 输入参数  : delta *del：                   delta结构体实例
			   decode_block *blk：            需要操作的block
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void decode_put_tail_to_head(delta *del,decode_block *blk){
    blk->PREV->NEXT=NULL;
    del->TAIL=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=del->HEAD;
    del->HEAD->PREV=blk;
    del->HEAD=blk;
    return;
}
/*****************************************************************************
 * 函 数 名  : decode_put_mid_to_head
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 把block链表中的中间一位放到第一位
 * 输入参数  : delta *del：                   delta结构体实例
			   decode_block *blk：            需要操作的block
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void decode_put_mid_to_head(delta *del,decode_block *blk){
    blk->PREV->NEXT=blk->NEXT;
    blk->NEXT->PREV=blk->PREV;
    blk->PREV=NULL;
    blk->NEXT=del->HEAD;
    del->HEAD->PREV=blk;
    del->HEAD=blk;
    return;
}
/*****************************************************************************
 * 函 数 名  : find_block_in_pool
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 在block链表中寻找给定block number的block
 * 输入参数  : delta *del：				delta结构体实例
			   uint32_t blk_no：		需要被寻找的blcok number
 * 输出参数  : 无
 * 返 回 值  : 找到则返回block指针，找不到则返回NULL
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
decode_block *find_block_in_pool(delta *del,uint32_t blk_no){
    for(int x=0;x<del->BLOCK_INPOOL;x++){
        if(del->DECODE_BLOCK_LIST[x].BLOCK_NUMBER==blk_no)return &del->DECODE_BLOCK_LIST[x];
    }
    return NULL;
}
/*****************************************************************************
 * 函 数 名  : decode_get_block
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 读取指定的block
 * 输入参数  : delta *del：				delta结构体实例
			   uint32_t blk_no：		需要被读取的blcok number
 * 输出参数  : 无
 * 返 回 值  : D_OK则读取成功，D_ERROR则读取失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT decode_get_block(delta *del,uint32_t blk_no){
    if(del->CURRENT_BLOCK->BLOCK_NUMBER==blk_no)return D_OK;
    decode_block *temp=find_block_in_pool(del, blk_no);
    if (temp==NULL) {
        size_t data_size;
        uint32_t data_position=(uint32_t)del->SOURCE_SEGMENT_POSITION+(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER)*blk_no;
        temp=del->TAIL;
        decode_put_tail_to_head(del,temp);
        temp->BLOCK_NUMBER=blk_no;
        if(blk_no==del->BLOCK_COUNT-1 && del->SOURCE_SEGMENT_LENGTH%(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER)!=0){
            data_size=del->SOURCE_SEGMENT_LENGTH%(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
        }else data_size=DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER;
        memset(temp->DATA, 0, DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
		read_bytes_at(&del->SOURCE_FILE,data_size,data_position,temp->DATA);
        temp->DATA_SIZE=data_size;
        del->CURRENT_BLOCK=temp;
    } else {
        if(temp==del->TAIL){
            decode_put_tail_to_head(del,temp);
        }else{
            decode_put_mid_to_head(del, temp);
        }
        del->CURRENT_BLOCK=temp;
    }

    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : copy_data
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 从源文件中向指定缓存区拷贝指定的数据到指定的位置
 * 输入参数  : delta *del：				delta结构体实例
			   dposition decode_posi：	缓存区的位置
			   dposition addr：			源文件地址
			   dsize siz：				需要拷贝的大小
 * 输出参数  :
 * 返 回 值  : D_OK成功，D_ERROR失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT copy_data(delta *del,dposition decode_posi, dposition addr,dsize siz){
    uint64_t init_blk_size=(del->BLOCK_COUNT==1)?del->SOURCE_SEGMENT_LENGTH:(DECODE_SOURCE_SIZE/DECODE_BLOCK_NUMBER);
    uint32_t blk_no=(uint32_t)addr/(init_blk_size);
    uint32_t local_position=(uint32_t)addr%(init_blk_size);
    dsize data_remain;
    if(addr>=del->SOURCE_SEGMENT_LENGTH){//scopy
        for (int x=0; x<siz; x++) {
            del->UPDATED_BUFFER[decode_posi+x]=del->UPDATED_BUFFER[addr-del->SOURCE_SEGMENT_LENGTH+x];
        }
    }else{
        while (siz) {//copy
            decode_get_block(del, blk_no);
            data_remain=del->CURRENT_BLOCK->DATA_SIZE-local_position;
            if(data_remain>=siz){
                memcpy(&del->UPDATED_BUFFER[decode_posi], &del->CURRENT_BLOCK->DATA[local_position], siz);
                siz=0;
            }else{
                memcpy(&del->UPDATED_BUFFER[decode_posi], &del->CURRENT_BLOCK->DATA[local_position], data_remain);
                siz-=data_remain;
                blk_no+=1;
                local_position=0;
                decode_posi+=data_remain;
            }
        }
    }

    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : set_HDR
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 设置delta结构体的HDR，HDR决定了这个delta文件是否使用了二次压缩和自定义指令表
 * 输入参数  : delta *del:  delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void set_HDR(delta *del){
    del->HDR_INDICATOR=read_byte(&del->DELTA_FILE);
	if(del->HDR_INDICATOR&(1<<2))del->DECODE_MODE=1;
	else if(del->HDR_INDICATOR&(1<<3)){//如果读到的模式为2和3，则设置目标文件的地址为源文件的地址，以达到直接覆盖源文件的功能

		del->DECODE_MODE=2;
		del->UPDATED_FILE.FileBeginAddr=GET_SOURCE_POSITION();
	}
	else {
		del->DECODE_MODE=3;
		del->UPDATED_FILE.FileBeginAddr=GET_SOURCE_POSITION();
	}
	
    return;
}
/*****************************************************************************
 * 函 数 名  : set_first_win
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 设置第一个目标窗口的解码位置
 * 输入参数  : delta *del: dalta结构体实例
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void set_first_win(delta *del){
    //del->DELTA_FILE.FileOffset=5;
    del->CURRENT_WIN_POSI=del->DELTA_FILE.FileOffset;//设置当前窗口的位置
    del->NEXT_WIN_POSI=0;//下一窗口的位置被暂时设置为0，后面解析窗口时会重新设置
    return;
}
/*****************************************************************************
<<<<<<< HEAD
 * 函 数 名  : set_file_size
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.10
* 函数功能  : 从delta文件中读取数据并设置各个文件的大小，如果从delta文件中读到的源文件大小与设备记录的源文件大小不同的话，则会返回错误，并终止升级
 * 输入参数  : delta *del: dalta结构体实例
 * 输出参数  : 无
 * 返 回 值  : D_OK成功，D_ERROR失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT set_file_size(delta *del){//设置源文件和目标文件大小
	del->UPDATED_SIZE=read_integer(&del->DELTA_FILE);
	del->SOURCE_SIZE=read_integer(&del->DELTA_FILE);
	//TODO: 这里需要与设备之前记录的源文件大小作比较，如果不一样则返回错误
	//if.......
	return D_OK;
}
/*****************************************************************************
=======
>>>>>>> board_code
 * 函 数 名  : decode_window
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 对一个窗口进行解码
 * 输入参数  : delta *del：delta实例
 * 输出参数  : 无
 * 返 回 值  : D_OK成功，D_ERROR失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT decode_window(delta *del){
    memset(&cache,0,sizeof(cache));//初始化地址缓存
    dfile *delfile=&del->DELTA_FILE;
    if(del->CURRENT_WIN_POSI!=delfile->FileOffset)return D_ERROR;
    set_win_header(del);
    init_source_lru(del);//初始化lru
    memset(del->UPDATED_BUFFER, 0, DEFAULT_UPDATED_WIN_SIZE);
    dposition curr_decode_position=0;
    //reading delta routine
    dposition inst_posi=del->CURRENT_INST_POSI;
    dposition addr_posi=del->CURRENT_ADDR_POSI;
    dposition data_posi=del->CURRENT_DATA_POSI;
    byte instcode,mode;
    int m;
    dposition addr;
    dsize size;
    while(inst_posi<del->CURRENT_ADDR_POSI){//
        instcode=read_byte_at(delfile, inst_posi);
        inst_posi++;
        switch (DeltaDefaultCodeTable[0][instcode]) {
            case COPY:
                //设置size
                if(DeltaDefaultCodeTable[2][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=delfile->FileOffset;
                }else{
                    size=DeltaDefaultCodeTable[2][instcode];
                }
                //设置addr
                mode=DeltaDefaultCodeTable[4][instcode];
                if(mode==VCD_SELF){
                    addr=read_integer_at(delfile, addr_posi);
                    addr_posi=delfile->FileOffset;
                }else if (mode==VCD_HERE){
                    addr=curr_decode_position+del->SOURCE_SEGMENT_LENGTH-read_integer_at(delfile, addr_posi);
                    addr_posi=delfile->FileOffset;
                }else if((m=mode-2)>=0 && m<s_near){
                    addr=cache.near[m]+read_integer_at(delfile, addr_posi);
                    addr_posi=delfile->FileOffset;
                }else{
                    m=mode-(2+s_near);
                    addr=cache.same[m*256+read_byte_at(delfile, addr_posi)];
                    addr_posi=delfile->FileOffset;
                }
                cache_update(&cache, addr);
                //读取数据
                copy_data(del, curr_decode_position, addr, size);
                curr_decode_position+=size;
                break;
            case ADD:
                //设置size
                if(DeltaDefaultCodeTable[2][instcode]==0){
                    size=read_integer_at(delfile, inst_posi);
                    inst_posi=delfile->FileOffset;
                }else{
                    size=DeltaDefaultCodeTable[2][instcode];
                }
                //读取数据
                read_bytes_at(delfile, size, data_posi, &del->UPDATED_BUFFER[curr_decode_position]);
                data_posi=delfile->FileOffset;
                curr_decode_position+=size;
                break;
            case RUN:
                //设置size
                size=read_integer_at(delfile, inst_posi);
                inst_posi=delfile->FileOffset;
                memset(&del->UPDATED_BUFFER[curr_decode_position], read_byte_at(delfile, data_posi), size);
                data_posi=delfile->FileOffset;
                curr_decode_position+=size;
                break;
            default:
                break;
        }
    }
	if(erase_page_at ( del->UPDATED_FILE.FileBeginAddr+del->UPDATED_FILE.FileOffset, DEFAULT_UPDATED_WIN_SIZE )==0){
		return D_ERROR;
	}
    write_data(&del->UPDATED_FILE,del->UPDATED_BUFFER, del->LENGTH_TARGET_WIN);
	delfile->FileOffset=del->NEXT_WIN_POSI;
    del->CURRENT_WIN_POSI=del->NEXT_WIN_POSI;
    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : set_win_header
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 设置窗口的header信息
 * 输入参数  : delta *del：delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void set_win_header(delta *del){
    dfile *delfile=&del->DELTA_FILE;
    del->WIN_INDICATOR=read_byte(delfile);
    del->SOURCE_SEGMENT_LENGTH=read_integer(delfile);
    del->SOURCE_SEGMENT_POSITION=read_integer(delfile);
    del->LENGTH_DELTA_ENCODE=read_integer(delfile);
    del->NEXT_WIN_POSI=delfile->FileOffset+del->LENGTH_DELTA_ENCODE;
    del->LENGTH_TARGET_WIN=read_integer(delfile);
    del->DELTA_INDICATOR=read_byte(delfile);
    del->LEN_DATA=read_integer(delfile);
    del->LEN_INST=read_integer(delfile);
    del->LEN_ADDR=read_integer(delfile);
    del->CURRENT_DATA_POSI=delfile->FileOffset;
    del->CURRENT_INST_POSI=del->CURRENT_DATA_POSI+del->LEN_DATA;
    del->CURRENT_ADDR_POSI=del->CURRENT_INST_POSI+del->LEN_INST;
}
/*****************************************************************************
 * 函 数 名  : init_delta
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 初始化delta解码，设置文件起始位置和大小
 * 输入参数  : delta *del：delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : 无
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
void init_delta(delta *del){

	dfile_open(&del->DELTA_FILE,GET_DELTA_POSITION());
	dfile_open(&del->SOURCE_FILE,GET_SOURCE_POSITION());
	dfile_open(&del->UPDATED_FILE,GET_UPDATED_POSITION());

}
/*****************************************************************************
 * 函 数 名  : verify_file
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 校验delta，文件，未来此处可能要添加md5校验
 * 输入参数  : delta *del：delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : D_OK成功，D_ERROR失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT verify_file(delta *del){//检验delta文件的magic bytes和版本信息，未来需要加入源文件的MD5和delta文件的MD5的校验

	uint8_t x;
	uint32_t read_size;
    dfile *delfile=&del->DELTA_FILE;
	byte md5_from_delta[16];
	byte md5_calculated[16];
	//检测源文件md5,delta文件md5
	MD5_CTX md5_checker;
	MD5Init(&md5_checker);
	uint32_t file_size;
	uint32_t file_posi=GET_DELTA_POSITION()+18;
	uint32_t calculated=0;
	for (x=0;x<16;x++){
		md5_from_delta[x]=read_byte(delfile);
	}
	file_size=(read_byte(delfile)<<8)|read_byte(delfile);
	while(calculated<file_size){
		read_size=delta_min(file_size,DEFAULT_UPDATED_WIN_SIZE);
		read_flash_at(calculated+file_posi,del->MOVE_DATA_BUFFER,read_size);
		MD5Update(&md5_checker,del->MOVE_DATA_BUFFER,read_size);
		calculated+=read_size;
	}
	MD5Final(md5_calculated, &md5_checker);
	for(x=0;x<16;x++){
		if(md5_calculated[x]!=md5_from_delta[x]){
			return D_ERROR;
		}
	}
	del->DELTA_SIZE=file_size;
	for (x=0;x<16;x++){
		md5_from_delta[x]=read_byte(delfile);
	}
	file_size=read_integer(delfile);
	file_posi=GET_SOURCE_POSITION();
	MD5Init(&md5_checker);
	calculated=0;
	while(calculated<file_size){
		read_size=delta_min(file_size,DEFAULT_UPDATED_WIN_SIZE);
		read_flash_at(calculated+file_posi,del->MOVE_DATA_BUFFER,read_size);
		MD5Update(&md5_checker,del->MOVE_DATA_BUFFER,read_size);
		calculated+=read_size;
	}
	MD5Final(md5_calculated, &md5_checker);
	for(x=0;x<16;x++){
		if(md5_calculated[x]!=md5_from_delta[x]){
			return D_ERROR;
		}
	}
	del->SOURCE_SIZE=file_size;

    if(read_byte(delfile)!=0xd6)return D_ERROR;
    if(read_byte(delfile)!=0xc3)return D_ERROR;
    if(read_byte(delfile)!=0xc4)return D_ERROR;
    if(read_byte(delfile)!=SOFTWARE_VERSION)return D_ERROR;
	del->UPDATED_SIZE=read_integer(&del->DELTA_FILE);
    return D_OK;
}
/*****************************************************************************
 * 函 数 名  : finalizer
 * 负 责 人  : 邹子豪
 * 创建日期  : 2021.1.9
 * 函数功能  : 解码完成后的收尾工作
 * 输入参数  : delta *del：delta结构体实例
 * 输出参数  : 无
 * 返 回 值  : D_OK成功，D_ERROR失败
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
D_RT finalizer(delta *del){
	uint32_t page;
	uint32_t begin_page=del->UPDATED_FILE.FileBeginAddr+
		((del->UPDATED_SIZE+(DEFAULT_UPDATED_WIN_SIZE-1))
		/DEFAULT_UPDATED_WIN_SIZE-1)*DEFAULT_UPDATED_WIN_SIZE+DEFAULT_UPDATED_WIN_SIZE;
	uint32_t end_page=del->SOURCE_FILE.FileBeginAddr+
		((del->SOURCE_SIZE+(DEFAULT_UPDATED_WIN_SIZE-1))
		/DEFAULT_UPDATED_WIN_SIZE-1)*DEFAULT_UPDATED_WIN_SIZE;
	for(page=begin_page;
		page<=end_page;
		page+=DEFAULT_UPDATED_WIN_SIZE){
		erase_page_at(page,DEFAULT_UPDATED_WIN_SIZE);
	}
	board_oriented_finalizer(del->DELTA_FILE.FileBeginAddr,
	del->DELTA_SIZE,
	del->UPDATED_FILE.FileBeginAddr,
	del->UPDATED_SIZE);
	return D_OK;
}
