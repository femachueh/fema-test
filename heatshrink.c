#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
// fema test

#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "heatshrink.h"

//#include "module_heap.h"
//#include "system_api.h"
#define MODULE_NAME     Compression
#define LOG_ERROR       printf
#define WINDOW_SIZE     HEATSHRINK_STATIC_WINDOW_BITS
#define LOOKAHEAD_SIZE  HEATSHRINK_STATIC_LOOKAHEAD_BITS

typedef struct _CompressCB
{
    uint8_t windowSize;
    uint8_t lookaheadSize;
    uint8_t verbose;

    CompressOperation op;
    union
    {
        heatshrink_encoder hse;
        heatshrink_decoder hsd;
    };
    uint8_t *inChunk;
    uint32_t inChunkSize;
    uint32_t inChunkOffset;

    uint8_t *inBuf;
    uint32_t inBufSize;
    uint8_t *outBuf;
    uint32_t outBufSize;
    CompressCallback callback;

    size_t  inTotal;
    size_t  outTotal;
} CompressCB;

static bool handleBeUsed;
static uint8_t inChunk[1 << HEATSHRINK_STATIC_WINDOW_BITS];
static CompressCB compressHandle = {
     .windowSize = WINDOW_SIZE, .lookaheadSize = LOOKAHEAD_SIZE, .verbose = 1,
     .inChunk = inChunk, .inChunkSize = ( 1 << HEATSHRINK_STATIC_WINDOW_BITS ), .inChunkOffset = 0
};

static bool EncodePolling(CompressHandle handle, uint32_t inSize, bool end);
static bool DecodePolling(CompressHandle handle, uint32_t inSize, bool end);
static void Report(CompressHandle handle);

/**
 * @brief   Constructor for compress
 * 
 * @param[in] inBuf         buffer of input data
 * @param[in] inBufSize     size of buffer of input data
 * @param[in] outBuf        buffer of output data
 * @param[in] outBufSize    size of buffer of output data
 * @param[in] outCallback   callback when output buffer full or 
 * @param[in] op            operation code - compress, decompress
 * @return CompressHandle   handle of compress, if NULL means no handle can be used
 */
CompressHandle CompressInit(uint8_t * inBuf, uint32_t inBufSize, uint8_t * outBuf, uint32_t outBufSize, CompressCallback outCallback, CompressOperation op)
{
    if ( !handleBeUsed && inBuf != NULL && inBufSize > 0 && outBuf != NULL && outBufSize > 0 && outCallback != NULL && ( op == COMPRESS_OP_COMPRESS || op == COMPRESS_OP_DECOMPRESS )  )
    {
        CompressHandle handle = &compressHandle;

        handleBeUsed = true;
        handle->op = op;
        handle->inBuf = inBuf;
        handle->inBufSize = inBufSize;
        handle->outBuf = outBuf;
        handle->outBufSize = outBufSize;
        handle->callback = outCallback;
        handle->inTotal = 0;
        handle->outTotal = 0;

        if ( op == COMPRESS_OP_COMPRESS )   heatshrink_encoder_reset( &handle->hse );
        if ( op == COMPRESS_OP_DECOMPRESS ) heatshrink_decoder_reset( &handle->hsd );

        return handle;
    }

    return NULL;
}

/**
 * @brief   Polling compress, may multiple times until feed all input data
 * 
 * @param[in] handle        compress handle
 * @param[in] inSize        size of data
 * @return CompressError    error code
 */
CompressError CompressPoll(CompressHandle handle, uint32_t inSize)
{
    if ( handle != &compressHandle || inSize > handle->inBufSize )
        return COMPRESS_ERR_MISUSE;

    if ( inSize > 0 )
    {
        if ( handle->op == COMPRESS_OP_COMPRESS && !EncodePolling( handle, inSize, false ) )    return COMPRESS_ERR_UNKNOWN;
        if ( handle->op == COMPRESS_OP_DECOMPRESS && !DecodePolling( handle, inSize, false ) )  return COMPRESS_ERR_UNKNOWN;
        handle->inTotal += inSize;
    }

    return COMPRESS_ERR_NONE;
}

/**
 * @brief   Destructor for compress
 * 
 * @param[in] handle        compress handle
 * @return CompressError    error code
 */
CompressError CompressFinish(CompressHandle handle)
{
    if ( handle != &compressHandle )
        return COMPRESS_ERR_MISUSE;

    if ( handle->op == COMPRESS_OP_COMPRESS && !EncodePolling( handle, 0, true ) )      return COMPRESS_ERR_UNKNOWN;
    if ( handle->op == COMPRESS_OP_DECOMPRESS && !DecodePolling( handle, 0, true ) )    return COMPRESS_ERR_UNKNOWN;

    if ( handle->verbose == 1 ) Report( handle );
    handleBeUsed = false;
    return COMPRESS_ERR_NONE;
}

/**
 * @brief   encode polling
 * 
 * @param[in] handle    compress handle
 * @param[in] inSize    size of data
 * @param[in] end       end of polling
 * @return bool         true : success, false : otherwise
 */
static bool EncodePolling(CompressHandle handle, uint32_t inSize, bool end)
{
    heatshrink_encoder * hse = &handle->hse;
    uint32_t inBufOffset = 0, dataRemain = inSize, cpySize;

    uint32_t pollOffset = 0, pollRemain = handle->outBufSize;
    size_t pollSize;
    HSE_poll_res pRes;

    while ( true )
    {
        cpySize = handle->inChunkSize - handle->inChunkOffset;
        if ( cpySize > dataRemain ) cpySize = dataRemain;

        memcpy( &handle->inChunk[handle->inChunkOffset], &handle->inBuf[inBufOffset], cpySize );
        inBufOffset += cpySize;
        dataRemain -= cpySize;
        handle->inChunkOffset += cpySize;
        if ( handle->inChunkOffset == 0 || ( !end && handle->inChunkOffset != handle->inChunkSize ) )   break;

        uint32_t sinkOffset = 0, sinkRemain = handle->inChunkOffset;
        size_t sinkSize = 0;
        HSE_sink_res sRes;

        while ( sinkOffset < handle->inChunkOffset )
        {
            sRes = heatshrink_encoder_sink( hse, &handle->inChunk[sinkOffset], sinkRemain, &sinkSize );
            if ( sRes < 0 ) { LOG_ERROR("encoder_sink failure\n"); return false; }
            sinkOffset += sinkSize;
            sinkRemain -= sinkSize;

            do
            {
                pRes = heatshrink_encoder_poll( hse, &handle->outBuf[pollOffset], pollRemain, &pollSize );
                if ( pRes < 0 ) { LOG_ERROR("encoder_poll failure\n"); return false; }
                pollOffset += pollSize;
                pollRemain -= pollSize;
                if ( pollRemain == 0 )
                {
                    if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
                    handle->outTotal += pollOffset;
                    pollOffset = 0, pollRemain = handle->outBufSize;
                }
            } while ( pRes == HSER_POLL_MORE );
        }
        handle->inChunkOffset = 0;
    }

    if ( end )
    {
        HSE_finish_res  fRes;

        do
        {
            fRes = heatshrink_encoder_finish( hse );
            if ( fRes < 0 ) { LOG_ERROR("encoder_finish failure\n"); return false; }
            pRes = heatshrink_encoder_poll( hse, &handle->outBuf[pollOffset], pollRemain, &pollSize );
            if ( pRes < 0 ) { LOG_ERROR("encoder_poll failure\n"); return false; }
            pollOffset += pollSize;
            pollRemain -= pollSize;
            if ( pollRemain == 0 )
            {
                if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
                handle->outTotal += pollOffset;
                pollOffset = 0, pollRemain = handle->outBufSize;
            }
        } while ( fRes != HSER_FINISH_DONE );
    }

    if ( pollOffset > 0 )
    {
        if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
        handle->outTotal += pollOffset;
    }

    return true;
}

/**
 * @brief   decode polling
 * 
 * @param[in] handle    compress handle
 * @param[in] inSize    size of data
 * @param[in] end       end of polling
 * @return bool         true : success, false : otherwise
 */
static bool DecodePolling(CompressHandle handle, uint32_t inSize, bool end)
{
    heatshrink_decoder * hsd = &handle->hsd;
    uint32_t inBufOffset = 0, dataRemain = inSize, cpySize;

    uint32_t pollOffset = 0, pollRemain = handle->outBufSize;
    size_t pollSize = 0;
    HSD_poll_res pRes;

    while ( true )
    {
        cpySize = handle->inChunkSize - handle->inChunkOffset;
        if ( cpySize > dataRemain ) cpySize = dataRemain;

        memcpy( &handle->inChunk[handle->inChunkOffset], &handle->inBuf[inBufOffset], cpySize );
        inBufOffset += cpySize;
        dataRemain -= cpySize;
        handle->inChunkOffset += cpySize;
        if ( handle->inChunkOffset == 0 || ( !end && handle->inChunkOffset != handle->inChunkSize ) )   break;

        uint32_t sinkOffset = 0, sinkRemain = handle->inChunkOffset;
        size_t sinkSize = 0;
        HSD_sink_res sRes;

        while ( sinkOffset < handle->inChunkOffset )
        {
            sRes = heatshrink_decoder_sink( hsd, &handle->inChunk[sinkOffset], sinkRemain, &sinkSize );
            if ( sRes < 0 ) { LOG_ERROR("decoder_sink failure\n"); return false; }
            sinkOffset += sinkSize;
            sinkRemain -= sinkSize;

            do
            {
                pRes = heatshrink_decoder_poll( hsd, &handle->outBuf[pollOffset], pollRemain, &pollSize );
                if ( pRes < 0 ) { LOG_ERROR("decoder_poll failure\n"); return false; }
                pollOffset += pollSize;
                pollRemain -= pollSize;
                if ( pollRemain == 0 )
                {
                    if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
                    handle->outTotal += pollOffset;
                    pollOffset = 0, pollRemain = handle->outBufSize;
                }
            } while ( pRes == HSDR_POLL_MORE );
        }
        handle->inChunkOffset = 0;
    }

    if ( end )
    {
        HSD_finish_res  fRes;

        do
        {
            fRes = heatshrink_decoder_finish( hsd );
            if ( fRes < 0 ) { LOG_ERROR("decoder_finish failure\n"); return false; }
            pRes = heatshrink_decoder_poll( hsd, &handle->outBuf[pollOffset], pollRemain, &pollSize );
            if ( pRes < 0 ) { LOG_ERROR("decoder_poll failure\n"); return false; }
            pollOffset += pollSize;
            pollRemain -= pollSize;
            if ( pollRemain == 0 )
            {
                if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
                handle->outTotal += pollOffset;
                pollOffset = 0, pollRemain = handle->outBufSize;
            }
        } while ( fRes != HSDR_FINISH_DONE );
    }

    if ( pollOffset > 0 )
    {
        if ( handle->callback( pollOffset ) != 0 )  { LOG_ERROR("callback failure\n"); return false; }
        handle->outTotal += pollOffset;
    }

    return true;
}

/**
 * @brief   report result
 * 
 * @param[in] handle compress handle
 */
static void Report(CompressHandle handle)
{
    size_t inb = handle->inTotal;
    size_t outb = handle->outTotal;
    LOG_INFO(
        "%0.2f %%\t %zd -> %zd (-w %u -l %u)\n",
        100.0 - (100.0 * outb) / inb, inb, outb,
        handle->windowSize, handle->lookaheadSize);
}


/** Test Code **/
#if 0
static uint8_t inBuf[7 * 1024], outBuf[7 * 1024];
static FS_FILE_TYPE *inFile, *outFile;

int CallbackTest(uint32_t outSize)
{
    size_t written = FS_FILE_WRITE(outBuf, 1, outSize, outFile);
    return 0;
}

void CompressTest(void)
{
    char *cInFileName = "\\test.bin", *cOutFileName = "\\test.z", *dInFileName = "\\test.zz", *dOutFileName = "\\test.bbin";
    uint32_t readSize;
    CompressHandle handle;
    TimeTick tick;

    LOG_INFO("Test compress\n");
    tick = TIME_GET_TICK();
    inFile = FS_FILE_OPEN(cInFileName, "rb");
    outFile = FS_FILE_OPEN(cOutFileName, "wb");
    if ( inFile == NULL || outFile == NULL )    { LOG_ERROR("File open fail\n"); return; }
    handle = CompressInit( inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), CallbackTest, COMPRESS_OP_COMPRESS );
    do
    {
        readSize = FS_FILE_READ(inBuf, 1, sizeof(inBuf), inFile);
        if ( CompressPoll( handle, readSize ) != COMPRESS_ERR_NONE )    return;
    } while ( readSize > 0 );
    if ( CompressFinish( handle ) != COMPRESS_ERR_NONE )    return;
    FS_FILE_CLOSE( inFile );
    FS_FILE_CLOSE( outFile );
    LOG_INFO("Test compress done %u ms\n", TIME_GET_TICK() - tick);

    LOG_INFO("Test decompress\n");
    tick = TIME_GET_TICK();
    inFile = FS_FILE_OPEN(dInFileName, "rb");
    outFile = FS_FILE_OPEN(dOutFileName, "wb");
    if ( inFile == NULL || outFile == NULL )    { LOG_ERROR("File open fail\n"); return; }
    handle = CompressInit( inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), CallbackTest, COMPRESS_OP_DECOMPRESS );
    do
    {
        readSize = FS_FILE_READ(inBuf, 1, sizeof(inBuf), inFile);
        if ( CompressPoll( handle, readSize ) != COMPRESS_ERR_NONE )    return;
    } while ( readSize > 0 );
    if ( CompressFinish( handle ) != COMPRESS_ERR_NONE )    return;
    FS_FILE_CLOSE( inFile );
    FS_FILE_CLOSE( outFile );
    LOG_INFO("Test decompress done %u ms\n", TIME_GET_TICK() - tick);
}
#endif
