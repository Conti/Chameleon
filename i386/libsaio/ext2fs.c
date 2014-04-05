/*
 *  ext2fs.c
 *  
 *
 *  Created by mackerintel on 1/26/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "libsaio.h"
#include "sl.h"
#include "ext2fs.h"

#define EX2ProbeSize	2048

bool EX2Probe (const void *buf)
{
	return (OSReadLittleInt16(buf+0x438,0)==0xEF53);
}

void EX2GetDescription(CICell ih, char *str, long strMaxLen)
{
	char * buf=malloc (EX2ProbeSize);
	str[0]=0;
	if (!buf) {
		return;
	}
	Seek(ih, 0);
	Read(ih, (long)buf, EX2ProbeSize);
	if (!EX2Probe (buf)) {
		free (buf);
		return;
	}
	if (OSReadLittleInt32 (buf+0x44c,0)<1) {
		free (buf);
		return;
	}
	str[strMaxLen]=0;
	strncpy (str, buf+0x478, MIN(strMaxLen, 16));
	free (buf);
}

long EX2GetUUID(CICell ih, char *uuidStr)
{
	uint8_t *b, *buf=malloc (EX2ProbeSize);
	if (!buf) {
		return -1;
	}
	Seek(ih, 0);
	Read(ih, (long)buf, EX2ProbeSize);
	if (!EX2Probe (buf)) {
		free (buf);
		return -1;
	}
	if (OSReadLittleInt32 (buf+0x44c,0)<1) {
		free (buf);
		return -1;
	}
	b=buf+0x468;
	sprintf(uuidStr,
		"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		b[0], b[1], b[2], b[3],
		b[4], b[5],
		b[6], b[7],
		b[8], b[9],
		b[10], b[11], b[12], b[13], b[14], b[15]);
	free (buf);
	return 0;
}
