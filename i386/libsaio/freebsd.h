#ifndef __LIBSAIO_FREEBSD_H
#define __LIBSAIO_FREEBSD_H

extern bool FreeBSDProbe (const void *buf);
extern void FreeBSDGetDescription(CICell ih, char *str, long strMaxLen);

#endif /* !__LIBSAIO_FREEBSD_H */
