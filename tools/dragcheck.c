/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * dragcheck [MODEID=$7A000001] [BACK=$7A000001] [SECS=60] : a test picture
 * for the compositor. Opens a screen in BACK with vertical stripes and one
 * in MODEID in front with horizontal stripes (16 rows each, pens 1-7), then
 * waits SECS seconds so the host can move the front screen (scrmove) and
 * read the card's picture back: above the front screen every row must be
 * the back screen's stripes, in its band the front's rows shifted by its
 * position. Needs dragging on in PrismPrefs.
 *
 * scrmove DY : MoveScreen the front screen by DY rows (same file, built as
 * its own program with -DSCRMOVE).
 */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#ifdef SCRMOVE
int main(int argc, char **argv)
{
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);
    if (!IntuitionBase || argc < 2)
        return 20;
    MoveScreen(IntuitionBase->FirstScreen, 0, (WORD)atoi(argv[1]));
    printf("scrmove: front screen at %d\n", IntuitionBase->FirstScreen->TopEdge);
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
#else
#define TEMPLATE "MODEID/K,BACK/K,SECS/K/N"

static ULONG hexarg(LONG a, ULONG def)
{
    return a ? strtoul((char *)a + (*(char *)a == '$'), NULL, 16) : def;
}

int main(void)
{
    LONG args[3] = { 0 };
    struct RDArgs *rd;
    ULONG front = 0x7A000001, back = 0x7A000001, secs = 60;
    struct Screen *bs, *fs;
    struct RastPort *rp;
    WORD x, y;

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
    if (!IntuitionBase || !GfxBase)
        return 20;
    if ((rd = ReadArgs((STRPTR)TEMPLATE, args, NULL))) {
        front = hexarg(args[0], front);
        back = hexarg(args[1], back);
        if (args[2]) secs = *(LONG *)args[2];
        FreeArgs(rd);
    }
    bs = OpenScreenTags(NULL, SA_DisplayID, back, SA_Depth, 8, SA_Title, (ULONG)"dragcheck back",
                        SA_Draggable, TRUE, TAG_DONE);
    fs = bs ? OpenScreenTags(NULL, SA_DisplayID, front, SA_Depth, 8, SA_Title, (ULONG)"dragcheck front",
                             SA_Draggable, TRUE, TAG_DONE) : NULL;
    if (!fs) {
        printf("dragcheck: can't open the screens\n");
        if (bs) CloseScreen(bs);
        return 10;
    }
    rp = &bs->RastPort;
    for (x = 0; x < bs->Width; x += 16) {
        SetAPen(rp, 1 + ((x >> 4) % 7));
        RectFill(rp, x, 0, x + 15 < bs->Width ? x + 15 : bs->Width - 1, bs->Height - 1);
    }
    rp = &fs->RastPort;
    for (y = 0; y < fs->Height; y += 16) {
        SetAPen(rp, 1 + ((y >> 4) % 7));
        RectFill(rp, 0, y, fs->Width - 1, y + 15 < fs->Height ? y + 15 : fs->Height - 1);
    }
    printf("dragcheck: back %ux%u, front %ux%u, %lu s\n", bs->Width, bs->Height, fs->Width, fs->Height,
           (unsigned long)secs);
    Delay(secs * 50);
    CloseScreen(fs);
    CloseScreen(bs);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
#endif
