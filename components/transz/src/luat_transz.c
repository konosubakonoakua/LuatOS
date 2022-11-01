#include "luat_transz.h"
#include "miniz.h"
#include <stdio.h>

#define TZ_MALLOC malloc
#define TZ_FREE   free
#define TZ_MEMCPY memcpy
#define TZ_MEMSET memset

// size_t cache_match = 0;

int luat_transz_read(luat_transz_data_t* data, uint8_t* buff, size_t offset, size_t len, luat_transz_cache_t* cache) {
    //int ret = 0;
    uint8_t* tmp = NULL;
    uint8_t* dst = buff;
    size_t block_index = 0;
    size_t tmp_size = 0;
    size_t block_size = 0;
    size_t origin_len = len;
    // 先防御一波
    if (data == NULL)
        return -1;
    if (buff == NULL)
        return -2;
    if (len == 0) {
        //printf("%s:%d what1\r\n", __FILE__, __LINE__);
        return 0;
    }
    if (offset >= data->total_size) {
        //printf("%s:%d what2 %d %d\r\n", __FILE__, __LINE__, offset, data->total_size);
        return 0;
    }

    if (offset + len >= data->total_size) {
        len = data->total_size - offset;
    }
    // printf("mode %d offset %d len %d\r\n", data->compress_mode, offset, len);
    if (data->compress_mode == TZ_COMPRESS_NONE) {
        // printf("data[0] %p\r\n", data->data[0]);
        // printf("data[0][8] %p\r\n", data->data[0][8]);
        TZ_MEMCPY(buff, data->datas + offset, len);
        return len;
    }

    block_size = data->block_size;

    // LLOGD("offset %08X len %08X", offset, len);
    if (cache == NULL) {
        tmp = TZ_MALLOC(block_size);
        if (tmp == NULL) {
            return -3;
        }
    }
    else {
        tmp = cache->buff;
    }
        
    uint16_t* flags = (uint16_t*)data->fragments;

    while (len > 0) {
        // 首先, 解压对应的块
        block_index = offset / block_size;
        size_t block_offset = 0;
        for (size_t i = 0; i < block_index; i++)
        {
            block_offset += flags[i];
        }
        //printf("block index %d offset %04X size %04X\r\n", block_index, block_offset, flags[block_index]);
        
        //printf("block_index %d\r\n", block_index);
        //printf("data->fragment_size %p\r\n", data->fragment_size);
        if (cache == NULL || cache->addr != block_index) {
            //printf("cache miss, do decompress\r\n");
            tmp_size = tinfl_decompress_mem_to_mem(tmp, block_size, data->datas + block_offset,
                                    flags[block_index]
                                    // 3766
                                    , TINFL_FLAG_PARSE_ZLIB_HEADER
                                    );
            if (cache != NULL) {
                // 无论解压是否成功,cache的数据已经污染,修改地址
                if (tmp_size == TINFL_DECOMPRESS_MEM_TO_MEM_FAILED)
                    cache->addr = -1;
                else
                    cache->addr = block_index;
            } 
        }
        else {
            //printf("cache hit, skip decompress\r\n");
            tmp_size = 0;
        }
        // 解压完成, 判断一下大小
        if (tmp_size == TINFL_DECOMPRESS_MEM_TO_MEM_FAILED) {
            if (cache != NULL)
                TZ_FREE(tmp); // 如果cache存在, 那么tmp就不是malloc的
            return -4;
        }
        // 开始分情况拷贝数据了
        if (offset % block_size == 0) {
            // 处于块的开头, 当前是对齐的状态
            //printf("%s:%d offset %d len %d\r\n", __FILE__, __LINE__, offset, len);
            if (len > block_size) {
                TZ_MEMCPY(dst, tmp, block_size);
                offset += block_size;
                dst += block_size;
                len -= block_size;
            }
            else {
                TZ_MEMCPY(dst, tmp, len);
                offset += len;
                dst += len;
                len = 0;
                break; // 结束了
            }
        }
        else {
            // 未对齐的状态, 拷贝并移动到对齐的位置
            size_t tmpoffset = offset % block_size;
            if (tmpoffset + len > block_size) {
                // 未对齐, 还跨块
                TZ_MEMCPY(dst, tmp + tmpoffset, block_size - tmpoffset);
                offset += block_size - tmpoffset;
                dst += block_size - tmpoffset;
                len -= block_size - tmpoffset;
            }
            else {
                // 未对齐但不跨块, 那肯定只有一个数据
                TZ_MEMCPY(dst, tmp + tmpoffset, len);
                offset += len;
                dst += len;
                len = 0;
                break; // 结束了
            }
        }
    }
    if (cache == NULL)
        TZ_FREE(tmp);
    // 一切正常
    //printf("%s:%d what3\r\n", __FILE__, __LINE__);
    return dst - buff;
}
