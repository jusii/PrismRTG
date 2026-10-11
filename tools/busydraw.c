/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * busydraw [SECS=10] [COLOURS] : keeps drawing text in a small window at the
 * top left of the Workbench screen (a clock or a busy shell, as far as the
 * pointer is concerned) and, with COLOURS, changes a colour of the screen
 * on every pass (colour cycling). For checking that the software pointer
 * stays on screen when what is drawn is nowhere near it.
 */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <stdio.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define TEMPLATE "SECS/K/N,COLOURS/S"

int main(void)
{
    LONG args[2] = { 0 };
    struct RDArgs *rd;
    ULONG secs = 10, vb0, n = 0;
    BOOL colours = FALSE;
    struct Window *w;
    char line[40];

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
    if (!IntuitionBase || !GfxBase)
        return 20;
    if ((rd = ReadArgs((STRPTR)TEMPLATE, args, NULL))) {
        if (args[0]) secs = *(LONG *)args[0];
        colours = args[1] != 0;
        FreeArgs(rd);
    }
    w = OpenWindowTags(NULL, WA_Left, 0, WA_Top, 20, WA_Width, 260, WA_Height, 120,
                       WA_Title, (ULONG)"busydraw", WA_DragBar, TRUE, WA_NoCareRefresh, TRUE, TAG_DONE);
    if (!w)
        return 10;
    vb0 = GfxBase->VBCounter;
    while (GfxBase->VBCounter - vb0 < secs * 50) {
        struct RastPort *rp = w->RPort;
        int k = sprintf(line, "line %lu        ", (unsigned long)n);
        SetAPen(rp, 1 + (n & 1));
        Move(rp, w->BorderLeft + 4, w->BorderTop + 10 + (WORD)((n % 9) * 10));
        Text(rp, line, k);
        if (colours)
            SetRGB32(&w->WScreen->ViewPort, 7, (n & 0xff) << 24, 0x80000000, 0x40000000);
        n++;
        WaitTOF();
    }
    printf("busydraw: %lu passes in %lu s\n", (unsigned long)n, (unsigned long)secs);
    CloseWindow(w);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
