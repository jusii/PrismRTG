/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * yuv.h - YUV 4:2:2 pixels to 0x00RRGGBB (the PF_YUV_* formats of
 * prismboard.h). Two pixels share a longword, so a pixel's U and V come
 * from the longword it sits in; the row base is longword aligned (sources
 * are an even number of pixels wide).
 */
#ifndef PRISM_YUV_H
#define PRISM_YUV_H

#include "prismboard.h"

static inline UBYTE yuv_clamp(LONG v)
{
    return v < 0 ? 0 : v > 255 ? 255 : v;
}

/* BT.601: CCIR ranges (Y 16..235, Cb/Cr 16..240) or full 0..255 */
static inline ULONG yuv_rgb(UBYTE fmt, const UBYTE *pixel)
{
    const UBYTE *q = (const UBYTE *)((ULONG)pixel & ~3UL);    /* the pair */
    BOOL second = ((ULONG)pixel & 2) != 0;
    LONG y, u, v, r, g, b;

    switch (fmt) {
    default:
    case PF_YUV_CGX:
    case PF_YUV_YUYV:   y = q[second ? 2 : 0]; u = q[1]; v = q[3]; break;
    case PF_YUV_P96:    y = q[second ? 0 : 2]; u = q[1]; v = q[3]; break;
    case PF_YUV_P96PC:  y = q[second ? 3 : 1]; u = q[2]; v = q[0]; break;
    case PF_YUV_P96PA:  y = q[second ? 1 : 0]; u = q[3]; v = q[2]; break;
    case PF_YUV_P96PAPC: y = q[second ? 2 : 3]; u = q[0]; v = q[1]; break;
    }
    u -= 128;
    v -= 128;
    if (fmt == PF_YUV_CGX) {
        LONG c = 298 * (y - 16) + 128;
        r = (c + 409 * v) >> 8;
        g = (c - 100 * u - 208 * v) >> 8;
        b = (c + 516 * u) >> 8;
    } else {
        r = y + ((359 * v) >> 8);
        g = y - ((88 * u + 183 * v) >> 8);
        b = y + ((454 * u) >> 8);
    }
    return ((ULONG)yuv_clamp(r) << 16) | ((ULONG)yuv_clamp(g) << 8) | yuv_clamp(b);
}

#endif
