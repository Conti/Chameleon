#ifndef __LIBSAIO_OPENBSD_H
#define __LIBSAIO_OPENBSD_H

extern bool OpenBSDProbe (const void *buf);
extern void OpenBSDGetDescription(CICell ih, char *str, long strMaxLen);

#endif /* !__LIBSAIO_OPENBSD_H */
