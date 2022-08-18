cmake_minimum_required (VERSION 2.8.12)
project (ethash C)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

set(HEADERS
    data_sizes.h
    endian.h
    ethash.h
    internal.h
    fnv.h
    )

set(SOURCES
    internal.c
    keccakf800.c
   )



add_library(ethash STATIC
    ${HEADERS}
    ${SOURCES}
    )
