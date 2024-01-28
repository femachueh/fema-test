
#ifndef HEATSHRINK_API_H
#define HEATSHRINK_API_H

#include "stdint.h"
#define LOG_ERROR printf
#define LOG_INFO printf
typedef struct _CompressCB *    CompressHandle;
typedef int (*CompressCallback)(uint32_t outSize);      ///< return 0 -> no error, otherwise -> error happened
typedef enum
{
    COMPRESS_OP_COMPRESS,
    COMPRESS_OP_DECOMPRESS
} CompressOperation;
typedef enum 
{ 
    COMPRESS_ERR_NONE,
    COMPRESS_ERR_MISUSE,
    COMPRESS_ERR_UNKNOWN
} CompressError;

CompressHandle CompressInit(uint8_t * inBuf, uint32_t inBufSize, uint8_t * outBuf, uint32_t outBufSize, CompressCallback outCallback, CompressOperation op);
CompressError CompressPoll(CompressHandle handle, uint32_t inSize); // may callback multiple times
CompressError CompressFinish(CompressHandle handle); // may callback multiple times
#endif
