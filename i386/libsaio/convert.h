/*
 * Convert.h
 *   Declare conversion utility functions
 *   --Rek
 */

#ifndef __CONVERT_H
#define __CONVERT_H
#include "libsaio.h"
#include "efi.h"

#define UUID_LEN	16

const char * getStringFromUUID(const EFI_CHAR8* uuid);
EFI_CHAR8*  getUUIDFromString(const char *source);
void *convertHexStr2Binary(const char *hexStr, int *outLength);
uint32_t ascii_hex_to_int(char *buff);

static inline uint16_t dp_swap16(uint16_t toswap)
{
    return (((toswap & 0x00FF) << 8) | ((toswap & 0xFF00) >> 8));
}

static inline uint32_t dp_swap32(uint32_t toswap)
{
    return  ((toswap & 0x000000FF) << 24) |
        ((toswap & 0x0000FF00) << 8 ) |
        ((toswap & 0x00FF0000) >> 8 ) |
        ((toswap & 0xFF000000) >> 24);
}	


#endif
