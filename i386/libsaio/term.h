/*
 *  term.h
 *  Chameleon
 *
 *  Created by JrCs on 30/08/11.
 *  Copyright 2011. All rights reserved.
 *
 */

#ifndef __LIBSAIO_TERM_H
#define __LIBSAIO_TERM_H

#define ASCII_KEY_MASK 0x7f
#define ASCII_KEY(x) ((x & ASCII_KEY_MASK))

#define KEY_BKSP    0x0008
#define KEY_TAB     0x0009
#define KEY_ENTER   0x000d
#define KEY_ESC     0x001b
#define KEY_PRTSC   0x002a
#define KEY_LEFT    0x4b00
#define KEY_RIGHT   0x4d00
#define KEY_UP      0x4800
#define KEY_DOWN    0x5000
#define KEY_HOME    0x4700
#define KEY_END     0x4f00
#define KEY_CENTER  0x4c00
#define KEY_INS     0x5200
#define KEY_DEL     0x5300
#define KEY_PGUP    0x4900
#define KEY_PGDN    0x5100
#define KEY_F1      0x3b00
#define KEY_F2      0x3c00
#define KEY_F3      0x3d00
#define KEY_F4      0x3e00
#define KEY_F5      0x3f00
#define KEY_F6      0x4000
#define KEY_F7      0x4100
#define KEY_F8      0x4200
#define KEY_F9      0x4300
#define KEY_F10     0x4400
#define KEY_F11     0x5700
#define KEY_F12     0x5800

// Key code for input that shouldn't echoed back
#define KEY_NOECHO  0xff00

/* Bitmasks for modifier keys */
#define STATUS_RSHIFT (1 << 0)
#define STATUS_LSHIFT (1 << 1)
#define STATUS_RCTRL  (1 << 2)
#define STATUS_RALT   (1 << 3)
#define STATUS_SCROLL (1 << 4)
#define STATUS_NUM    (1 << 5)
#define STATUS_CAPS   (1 << 6)
#define STATUS_LCTRL  (1 << 8)
#define STATUS_LALT   (1 << 9)

#endif /* !__LIBSAIO_TERM_H */
