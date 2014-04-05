#include "libsaio.h"
#include "sl.h"
#include "openbsd.h"

#define OpenBSDProbeSize	2048
#define DISKMAGIC	((uint32_t)0x82564557)	/* The disk magic number */

bool OpenBSDProbe (const void *buf)
{
	return (OSReadLittleInt32(buf+0x200,0) == DISKMAGIC);
}
void OpenBSDGetDescription(CICell ih, char *str, long strMaxLen)
{
	char * buf=malloc(OpenBSDProbeSize);
	str[0]=0;
	if (!buf)
		return;
	Seek(ih, 0);
	Read(ih, (long)buf, OpenBSDProbeSize);
	if (!OpenBSDProbe (buf))
	{
		free (buf);
		return;
	}
	if (OSReadLittleInt32 (buf+0x44c,0)<1)
	{
		free (buf);
		return;
	}
	str[strMaxLen]=0;
	strncpy (str, buf+0x478, MIN (strMaxLen, 32));
	free (buf);
}
