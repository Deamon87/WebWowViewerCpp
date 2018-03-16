cmake_minimum_required(VERSION 3.6)

project(bzip)

set(SOURCE_FILES
        blocksort.c
        huffman.c
        crctable.c
        randtable.c
        compress.c
        decompress.c
        bzlib.c)

add_library(bz2 ${SOURCE_FILES})