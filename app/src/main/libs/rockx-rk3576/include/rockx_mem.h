#ifndef _ROCKX_MEM_H_
#define _ROCKX_MEM_H_

#include "rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate memory.
 * 
 * @param tag 
 * @param size 
 * @param type 
 * @param mem 
 * @return int 
 */
int RockXMemAlloc(const char* tag, size_t size, RockXMemType type, RockXMemoryBuffer* mem);

/**
 * @brief Free memory.
 * 
 * @param mem 
 * @return int 
 */
int RockXMemFree(RockXMemoryBuffer* mem);

/**
 * @brief Sync memory.
 * 
 * @param mem 
 * @return int 
 */
int RockXMemSync(RockXMemoryBuffer* mem);

/**
 * @brief Free memory by virtual address.
 * 
 * @param virtAddr 
 * @return int 
 */
int RockXMemFreeByVirtAddr(void* virtAddr);

/**
 * @brief Dump all memory.
 * 
 * @return int 
 */
int RockXMemDumpAll();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif