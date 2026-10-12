/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * vlayertest - a video layer (cgxvideo.library) with colour bars.
 *
 *   vlayertest [SECS=10] [FMT=YCBCR|YUV|RGB15|RGB16] [PUBSCREEN=name]
 *
 * Opens a 336x256 window on the public screen, creates a 160x120 video
 * layer of the format asked for, attaches it with a 8-pixel indent, and
 * writes eight colour bars (white, yellow, cyan, green, magenta, red,
 * blue, black - the standard order) into it through LockVLayer /
 * UnlockVLayer, then holds the window for SECS seconds. A test reads the
 * screen and expects the bars at the layer's place; the window's own
 * background is the colour-key pen the library handed out.
 */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdio.h>
#include <string.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *CGXVideoBase;

#define VOA_LeftIndent   0x88000001
#define VOA_RightIndent  0x88000002
#define VOA_TopIndent    0x88000003
#define VOA_BottomIndent 0x88000004
#define VOA_SrcType      0x88000005
#define VOA_SrcWidth     0x88000006
#define VOA_SrcHeight    0x88000007
#define VOA_Error        0x88000008
#define VOA_UseColorKey  0x88000009
#define VOA_BaseAddress  0x88000030
#define VOA_ColorKeyPen  0x88000031
#define VOA_Modulo       0x88000038
#define SRCFMT_YUV16     0
#define SRCFMT_YCbCr16   1
#define SRCFMT_RGB15     2
#define SRCFMT_RGB16     3

/* cgxvideo.library through tools/cgxvideo_stubs.S (no headers for it in this NDK) */
APTR  CreateVLayerHandleTagList(struct Screen *s, struct TagItem *tags);
void  DeleteVLayerHandle(APTR h);
ULONG AttachVLayerTagList(APTR h, struct Window *w, struct TagItem *tags);
ULONG DetachVLayer(APTR h);
ULONG GetVLayerAttr(APTR h, ULONG attr);
ULONG LockVLayer(APTR h);
void  UnlockVLayer(APTR h);
void  SetVLayerAttrTagList(APTR h, struct TagItem *tags);

/* BT.601 studio range for the CCIR format, full range for YUV16 */
static void rgb_to_yuv(ULONG rgb, BOOL ccir, UBYTE *y, UBYTE *u, UBYTE *v)
{
    LONG r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255;
    if (ccir) {
        *y = ( 66 * r + 129 * g +  25 * b + 128 + 4096) >> 8;
        *u = (-38 * r -  74 * g + 112 * b + 128 + 32768) >> 8;
        *v = (112 * r -  94 * g -  18 * b + 128 + 32768) >> 8;
    } else {
        *y = (77 * r + 150 * g + 29 * b) >> 8;
        *u = ((-43 * r - 85 * g + 128 * b) >> 8) + 128;
        *v = ((128 * r - 107 * g - 21 * b) >> 8) + 128;
    }
}

static const ULONG bars[8] = { 0xffffff, 0xffff00, 0x00ffff, 0x00ff00, 0xff00ff, 0xff0000, 0x0000ff, 0x000000 };

int main(void)
{
    static const char *template = "SECS/K/N,FMT/K,PUBSCREEN/K";
    LONG args[3] = { 0 };
    struct RDArgs *rda;
    struct Screen *scr = NULL;
    struct Window *win = NULL;
    APTR vl = NULL;
    ULONG secs = 10, type = SRCFMT_YCbCr16, err = 0, pen;
    struct TagItem ctags[6], atags[3];
    UWORD W = 160, H = 120, x, y;
    int rc = 20;

    if (!(rda = ReadArgs((STRPTR)template, args, NULL))) {
        PrintFault(IoErr(), (STRPTR)"vlayertest");
        return 20;
    }
    if (args[0]) secs = *(LONG *)args[0];
    if (args[1]) {
        const char *f = (const char *)args[1];
        type = !stricmp(f, "YUV") ? SRCFMT_YUV16 : !stricmp(f, "RGB15") ? SRCFMT_RGB15 :
               !stricmp(f, "RGB16") ? SRCFMT_RGB16 : SRCFMT_YCbCr16;
    }
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    CGXVideoBase = OpenLibrary((STRPTR)"cgxvideo.library", 41);
    if (!IntuitionBase || !GfxBase) goto out;
    if (!CGXVideoBase) { printf("vlayertest: no cgxvideo.library\n"); goto out; }
    if (!(scr = LockPubScreen((STRPTR)args[2]))) { printf("vlayertest: no public screen\n"); goto out; }

    ctags[0].ti_Tag = VOA_SrcType;     ctags[0].ti_Data = type;
    ctags[1].ti_Tag = VOA_SrcWidth;    ctags[1].ti_Data = W;
    ctags[2].ti_Tag = VOA_SrcHeight;   ctags[2].ti_Data = H;
    ctags[3].ti_Tag = VOA_UseColorKey; ctags[3].ti_Data = TRUE;
    ctags[4].ti_Tag = VOA_Error;       ctags[4].ti_Data = (ULONG)&err;
    ctags[5].ti_Tag = TAG_DONE;
    if (!(vl = CreateVLayerHandleTagList(scr, ctags))) {
        printf("vlayertest: CreateVLayerHandle failed, error %lu\n", (unsigned long)err);
        goto out;
    }
    printf("vlayertest: handle %lx\n", (unsigned long)vl); fflush(stdout);
    pen = GetVLayerAttr(vl, VOA_ColorKeyPen);
    printf("vlayertest: key pen %ld\n", (long)pen); fflush(stdout);
    win = OpenWindowTags(NULL, WA_Left, 40, WA_Top, 40, WA_Width, 336, WA_Height, 256,
                         WA_Title, (ULONG)"vlayertest", WA_PubScreen, (ULONG)scr,
                         WA_Flags, WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE | WFLG_SMART_REFRESH,
                         WA_IDCMP, IDCMP_CLOSEWINDOW, TAG_DONE);
    if (!win) { printf("vlayertest: no window\n"); goto out; }
    SetAPen(win->RPort, (LONG)pen < 0 ? 0 : pen);
    RectFill(win->RPort, win->BorderLeft, win->BorderTop, win->Width - win->BorderRight - 1,
             win->Height - win->BorderBottom - 1);
    atags[0].ti_Tag = VOA_LeftIndent; atags[0].ti_Data = 8;
    atags[1].ti_Tag = VOA_TopIndent;  atags[1].ti_Data = 8;
    atags[2].ti_Tag = TAG_DONE;
    if (AttachVLayerTagList(vl, win, atags)) { printf("vlayertest: AttachVLayer failed\n"); goto out; }

    if (LockVLayer(vl)) {
        UBYTE *base;
        ULONG mod;
        printf("vlayertest: locked\n"); fflush(stdout);
        base = (UBYTE *)GetVLayerAttr(vl, VOA_BaseAddress);
        mod = GetVLayerAttr(vl, VOA_Modulo);
        printf("vlayertest: layer at %lx, modulo %lu, key pen %ld, type %lu\n", (unsigned long)base,
               (unsigned long)mod, (long)pen, (unsigned long)type);
        fflush(stdout);
        for (y = 0; y < H; y++) {
            UBYTE *row = base + (ULONG)y * mod;
            for (x = 0; x < W; x += 2) {
                ULONG rgb = bars[x * 8 / W];
                if (type <= SRCFMT_YCbCr16) {
                    UBYTE yy, u, v;
                    rgb_to_yuv(rgb, type == SRCFMT_YCbCr16, &yy, &u, &v);
                    row[x * 2] = yy; row[x * 2 + 1] = u; row[x * 2 + 2] = yy; row[x * 2 + 3] = v;
                } else {
                    UWORD r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255, px;
                    px = type == SRCFMT_RGB16 ? ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                                              : ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
                    px = (px << 8) | (px >> 8);           /* PC byte order */
                    *(UWORD *)(row + x * 2) = px; *(UWORD *)(row + x * 2 + 2) = px;
                }
            }
        }
        printf("vlayertest: written\n"); fflush(stdout);
        UnlockVLayer(vl);
        printf("vlayertest: unlocked\n"); fflush(stdout);
    }
    printf("vlayertest: bars shown for %lu s\n", (unsigned long)secs);
    Delay(secs * 50);
    rc = 0;
out:
    if (vl) DeleteVLayerHandle(vl);
    if (win) CloseWindow(win);
    if (scr) UnlockPubScreen(NULL, scr);
    if (CGXVideoBase) CloseLibrary(CGXVideoBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    FreeArgs(rda);
    return rc;
}
