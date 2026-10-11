/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * movebench [MODEID=$7A000001] [BACK=$7A000001] [SECS=5] : the cost of a
 * screen being dragged. Opens a screen in BACK, one in MODEID in front of
 * it, moves the front one down 100 rows, then for SECS seconds moves it a
 * row up or down every frame (as a drag does) while counting a Move()
 * loop: the count is the CPU a program still gets. The same count with
 * the screen standing still comes first, for comparison. Needs dragging on
 * in PrismPrefs (otherwise the screen does not move).
 */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define TEMPLATE "MODEID/K,BACK/K,SECS/K/N"

static struct Screen *open_screen(ULONG id, const char *title)
{
    return OpenScreenTags(NULL, SA_DisplayID, id, SA_Depth, 8, SA_Title, (ULONG)title,
                          SA_Draggable, TRUE, SA_ShowTitle, TRUE, SA_Quiet, FALSE, TAG_DONE);
}

static void paint(struct Screen *s)
{
    struct RastPort *rp = &s->RastPort;
    WORD x;
    for (x = 0; x < s->Width; x += 16) {
        SetAPen(rp, 1 + ((x >> 4) & 7));
        RectFill(rp, x, 12, x + 15 < s->Width ? x + 15 : s->Width - 1, s->Height - 1);
    }
}

/* Move() calls in `secs` seconds; moving: also MoveScreen a row every frame */
static ULONG count(struct Screen *s, int secs, BOOL moving, ULONG *moves)
{
    ULONG vb0 = GfxBase->VBCounter, last = vb0, n = 0, m = 0;
    WORD dir = 1, at = 0;
    struct RastPort *rp = &s->RastPort;
    while (GfxBase->VBCounter - vb0 < (ULONG)secs * 50) {
        int i;
        for (i = 0; i < 50; i++)
            Move(rp, (WORD)(n++ & 255), 20);
        if (moving && GfxBase->VBCounter != last) {
            last = GfxBase->VBCounter;
            if (at >= 40) dir = -1;
            if (at <= -40) dir = 1;
            MoveScreen(s, 0, dir);
            at += dir;
            m++;
        }
    }
    if (moves) *moves = m;
    return n;
}

int main(void)
{
    LONG args[3] = { 0 };
    struct RDArgs *rd;
    ULONG front = 0x7A000001, back = 0x7A000001, secs = 5, still, moving, moves;
    struct Screen *bs, *fs;

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
    if (!IntuitionBase || !GfxBase)
        return 20;
    rd = ReadArgs((STRPTR)TEMPLATE, args, NULL);
    if (rd) {
        if (args[0]) front = strtoul((char *)args[0] + (*(char *)args[0] == '$'), NULL, 16);
        if (args[1]) back = strtoul((char *)args[1] + (*(char *)args[1] == '$'), NULL, 16);
        if (args[2]) secs = *(LONG *)args[2];
        FreeArgs(rd);
    }
    bs = open_screen(back, "movebench back");
    fs = bs ? open_screen(front, "movebench front") : NULL;
    if (!fs) {
        printf("movebench: can't open the screens\n");
        if (bs) CloseScreen(bs);
        return 10;
    }
    paint(bs);
    paint(fs);
    MoveScreen(fs, 0, 100);
    Delay(50);
    still = count(fs, secs, FALSE, NULL);
    moving = count(fs, secs, TRUE, &moves);
    printf("movebench: front $%08lx %ux%u, back $%08lx %ux%u, front at %d\n",
           (unsigned long)front, fs->Width, fs->Height, (unsigned long)back, bs->Width, bs->Height,
           fs->TopEdge);
    printf("  standing still: %lu Move()/s\n", (unsigned long)(still / secs));
    printf("  moving a row a frame: %lu Move()/s (%lu%% of still), %lu moves in %lu s\n",
           (unsigned long)(moving / secs), (unsigned long)(still ? moving * 100 / still : 0),
           (unsigned long)moves, (unsigned long)secs);
    CloseScreen(fs);
    CloseScreen(bs);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
