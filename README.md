# Delta
Delta compression for device with limited resource. I designed this software for enabling delta compression for SoC devices, which typically have limited resource and small package data. Though using a naïve greedy algorithm, this software can be useful to wide-range firmware update for IoT networks. 

**NOTE: :clown_face: This software was designed for small size file(typically <1M, because the maximal flash size of stm32 is 1M), which will perform a resource-consuming algorithm. APPLYING THIS SOFTWARE FOR A LARGE FILE CAN RESULT IN HUGE MEMORY USAGE AND LONG EXECUTION TIME** 

**Inspired** by 

Xdelta3(http://xdelta.org/)

and GOOGLE's

open-vfdiff(https://github.com/google/open-vcdiff)

## FEATURES:

1. Adapted for devices with limited resources, ones with small memory, limited network bandwidth and small storage unit.

   1. static memory usage. the smallest memory usage can be optimised to (flash page size)*2
   2. using greedy algorithm to perform maximum compression rate.
   3. provide three compression modes with different storage usage. Flexible for devices with different storage units.

2. formatted in VCdiff (detail: https://tools.ietf.org/html/rfc3284)

3. Three compression mode:

   1. decode target file in new storage space
   2. decode target file directly upon the storage space of source file.
   3. decode target file upon the storage space of source file with a backoff algorithm.

   Compression rate rank:

   i≈iii>ii

   Storage usage:

   i>iii>ii

## USAGE:

##### BUILD:

to build the project, you need to install CMake.

in the directory "Delta", create a new folder "build".

in the "build" folder, execute command `cmake ..`

execute command `make`

**ENCODE:**

```
./delta -e source_file_name target_file_name delta_file_name mode(-1/-2/-3) target_window_size(this size should be equal to the page size of the flash in target device)
```

**DECODE(on pc):**

```
./delta -d delta_file_name source_file_name target_window_size(this size should be equal to the page size of the flash in target device)
```

**To perform decode on SoC device, modify the source files under dir "BOARD_CODE"**

*you need somehow record your delta file size*

1, locate to the definition of the macro "DEFAULT_UPDATED_WIN_SIZE" in the file "BOARD_CODE.h".  Here I assign it to "FLASH_PAGE_SIZE", which is a macro defined in stm32 flash driver lib. Replace the value based on the actual page size of the flash in your target device. Next, you need to adjust the macros "DEFAULT_SOURCE_DATA_POSITION", "DEFAULT_DELTA_DATA_POSITION" and "DEFAULT_UPDATED_DATA_POSITION" to the flash location you wish to save your delta file, source file and target file

2, open the file "BOARD_CODE.c"; locate to the function group "文件操作函数(file io functions)". Here you need to rewrite these function based on your device library.

3, locate to the function "init_delta". Here you need to assign the real delta file size to the field "del->DELTA_SIZE". I use "DEBUG_DELTA_SIZE" just for debugging.

4, program your source file into flash at the location you defined at "DEFAULT_SOURCE_DATA_POSITION", and program your delta file at the location you defined at "DEFAULT_DELTA_DATA_POSITION". 

5, finally, write a test function. it can be simply as 

```c
int main(void){
    DECODER();
    while(1);
}
```

6, after executing the program, check your flash space using programmer tool (I use SEGGER flash, but it can be different based on your platform). export the target file (if you use mode 1, the target file will be decoded at the "DEFAULT_UPDATED_DATA_POSITION", else if you use mode 2 or 3, you will find your target file at the location of your original source file). Use file comparison tool to check the result.

**************************************************************

In order to implement a complete functionalities of a software upgrader. Some other features not included in my code, but you need to do it yourself, are: 1, a bootloader program. 2, a IAP program that integrates my decoder functionality. 

## OPEN SOURCE SOFTWARE USAGE

[mattsta/crcspeed: This make CRC be fast. Included implementations: CRC-64-Jones and CRC-16-CCITT (github.com)](https://github.com/mattsta/crcspeed)

[troydhanson/uthash: C macros for hash tables and more (github.com)](https://github.com/troydhanson/uthash)
## LICENSE

MIT(https://www.mit.edu/~amini/LICENSE.md)

