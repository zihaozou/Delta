# Delta
Delta compression for device with limited resource. I designed this software for enabling delta compression for SoC devices, which typically have limited resource and small package data. This software can be applied to wide-range firmware update for IoT networks. 

## FEATURES:

1. Adapted for devices with limited resources, ones with small memory, limited network bandwidth and small storage unit.

   1. static memory usage. the smallest memory usage can be optimised to (flash page size)*2
   2. using greedy algorithm to perform maximum compression rate.
   3. provide three compression modes with different storage usage. Flexible for devices with different storage units.

2. formatted in VCdiff (detail: https://tools.ietf.org/html/rfc3284)

3. Three compression mode:

   1. decode target file in new storage space
   2. decode target file directly upon the storage space of source file.
   3. decode target file upon the storage space of source file, but source file moves backward to perform better compression rate

   Compression rate rank:

   1≈3>2

   Storage usage:

   1>3>2

## USAGE:

ENCODE:

```
./delta -e source_file_name target_file_name delta_file_name mode(-1/-2/-3) target_window_size(this size should be equal to the page size of the flash in target device)
```

DECODE(on pc):

```
./delta -d delta_file_name source_file_name target_window_size(this size should be equal to the page size of the flash in target device)
```

To perform decode on SoC device, add the code 
