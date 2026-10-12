/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * bigpointer - a 64x64 pointer on a window, for a test of pointers larger
 * than a board's sprite.
 *
 *   bigpointer [SECS=10] [SIZE=64]
 *
 * Opens a window on the Workbench and gives it a solid square pointer of
 * SIZE pixels (two planes, pen 1 = the screen's sprite colour 17) through
 * pointerclass (V39), then holds it for SECS seconds. A board whose sprite
 * is smaller than SIZE must show it through PrismRTG's software pointer.
 */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/pointerclass.h>
#include <graphics/gfx.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdio.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

int main(void)
{
    static const char *template = "SECS/K/N,SIZE/K/N";
    LONG args[2] = { 0 };
    struct RDArgs *rda;
    struct Window *win = NULL;
    struct BitMap *bm = NULL;
    APTR ptr = NULL;
    ULONG secs = 10, size = 64, x, y;
    int rc = 20;

    if (!(rda = ReadArgs((STRPTR)template, args, NULL))) {
        PrintFault(IoErr(), (STRPTR)"bigpointer");
        return 20;
    }
    if (args[0]) secs = *(LONG *)args[0];
    if (args[1]) size = *(LONG *)args[1];
    if (size < 16) size = 16;
    if (size > 64) size = 64;
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    if (!IntuitionBase || !GfxBase) goto out;
    /* a solid square in pen 1, with a one-pixel pen-2 frame so its edge
     * can be seen against anything */
    if (!(bm = AllocBitMap(size, size, 2, BMF_CLEAR, NULL))) goto out;
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++) {
            BOOL edge = x == 0 || y == 0 || x == size - 1 || y == size - 1;
            UBYTE *p0 = bm->Planes[0] + y * bm->BytesPerRow + (x >> 3);
            UBYTE *p1 = bm->Planes[1] + y * bm->BytesPerRow + (x >> 3);
            if (edge) *p1 |= 0x80 >> (x & 7);
            else      *p0 |= 0x80 >> (x & 7);
        }
    win = OpenWindowTags(NULL, WA_Left, 100, WA_Top, 100, WA_Width, 400, WA_Height, 300,
                         WA_Title, (ULONG)"bigpointer", WA_Flags, WFLG_DRAGBAR | WFLG_ACTIVATE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW, TAG_DONE);
    if (!win) { printf("bigpointer: no window\n"); goto out; }
    ptr = NewObject(NULL, (STRPTR)"pointerclass", POINTERA_BitMap, (ULONG)bm, POINTERA_WordWidth, (size + 15) / 16,
                    POINTERA_XOffset, 0, POINTERA_YOffset, 0,
                    POINTERA_XResolution, POINTERXRESN_SCREENRES, POINTERA_YResolution, POINTERYRESN_SCREENRES,
                    TAG_DONE);
    if (!ptr) { printf("bigpointer: no pointer object\n"); goto out; }
    SetWindowPointer(win, WA_Pointer, (ULONG)ptr, TAG_DONE);
    printf("bigpointer: %lux%lu pointer set for %lu s\n", (unsigned long)size, (unsigned long)size, (unsigned long)secs);
    fflush(stdout);
    Delay(secs * 50);
    SetWindowPointer(win, WA_Pointer, 0, TAG_DONE);
    rc = 0;
out:
    if (ptr) DisposeObject(ptr);
    if (win) CloseWindow(win);
    if (bm) FreeBitMap(bm);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    FreeArgs(rda);
    return rc;
}
