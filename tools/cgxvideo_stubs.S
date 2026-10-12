| SPDX-License-Identifier: GPL-3.0-only
| Copyright (C) 2026 Laine Jones
| cgxvideo_stubs.S - C-callable stubs for cgxvideo.library (no headers for it
| in this NDK). Arguments on the stack, the library base in _CGXVideoBase.

        .text
        .globl  _CGXVideoBase

        .macro  STUB name, off, nargs
        .globl  _\name
_\name:
        move.l  a6,-(sp)
        move.l  _CGXVideoBase,a6
        move.l  8(sp),a0
        .if \nargs > 1
        move.l  12(sp),a1
        .endif
        .if \nargs > 2
        move.l  a2,-(sp)
        move.l  20(sp),a2
        .endif
        jsr     \off(a6)
        .if \nargs > 2
        move.l  (sp)+,a2
        .endif
        move.l  (sp)+,a6
        rts
        .endm

        STUB CreateVLayerHandleTagList, -30, 2
        STUB DeleteVLayerHandle,        -36, 1
        STUB AttachVLayerTagList,       -42, 3
        STUB DetachVLayer,              -48, 1
        STUB LockVLayer,                -60, 1
        STUB UnlockVLayer,              -66, 1
        STUB SetVLayerAttrTagList,      -72, 2

| GetVLayerAttr(handle a0, attr d0)
        .globl  _GetVLayerAttr
_GetVLayerAttr:
        move.l  a6,-(sp)
        move.l  _CGXVideoBase,a6
        move.l  8(sp),a0
        move.l  12(sp),d0
        jsr     -54(a6)
        move.l  (sp)+,a6
        rts
