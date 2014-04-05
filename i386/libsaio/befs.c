/*
 *  befs.c
 *  
 *
 *  Created by scorpius
 *  Copyright 2010
 *
 */

#include "libsaio.h"
#include "sl.h"
#include "befs.h"

#define BeFSProbeSize	2048

#define SUPER_BLOCK_MAGIC1 0x42465331 /* BFS1 */ 
#define SUPER_BLOCK_MAGIC2 0xdd121031 
#define SUPER_BLOCK_MAGIC3 0x15b6830e

/* Find BeFS signature */
bool BeFSProbe (const void *buf)
{
	return (OSReadLittleInt32(buf+0x220,0) == SUPER_BLOCK_MAGIC1);
}

/* Find BeFS volume label */
void BeFSGetDescription(CICell ih, char *str, long strMaxLen)
{
	char * buf=malloc (BeFSProbeSize);
	str[0]=0;
	if (!buf) {
		return;
	}
	Seek(ih, 0);
	Read(ih, (long)buf, BeFSProbeSize);
	if (!BeFSProbe (buf))
	{
		free (buf);
		return;
	}
	str[strMaxLen]=0;
	strncpy (str, buf+0x200, MIN (strMaxLen, 32));
	free (buf);
}
