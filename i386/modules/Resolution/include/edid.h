/*
 *  edid.h
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */

#ifndef _EDID_H_
#define _EDID_H_

#include "libsaio.h"

#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126

char* readEDID();
int getEDID( void * edidBlock, UInt8 block);
void getResolution(UInt32* x, UInt32* y, UInt32* bp);

#endif /* _EDID_H_ */