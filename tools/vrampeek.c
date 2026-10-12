/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * vrampeek - how much of a ZZ9000's video memory the bus reaches.
 *
 *   vrampeek [MB/N]
 *
 * Finds the board (manufacturer $6D6E), and for a set of offsets from the
 * start of its video memory (board base + $10000) writes a word, reads it
 * back and reports: a mapped offset reads back what was written, an
 * unmapped one reads the bus's idle value. Restores each word. Not for
 * use while PrismD shows a screen from that memory (it would scribble on
 * it - run it before PrismD, or accept a few dots).
 */
#include <exec/types.h>
#include <libraries/configvars.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>
#include <stdio.h>

struct ExpansionBase *ExpansionBase;

int main(void)
{
    static const ULONG offs[] = { 0, 0x100000, 0x1D0000, 0x1E0000, 0x1F0000, 0x200000, 0x210000,
                                  0x300000, 0x400000, 0x800000, 0xC00000, 0xDE0000 };
    struct ConfigDev *cd = NULL;
    volatile UWORD *p;
    UWORD i, was, got;

    if (!(ExpansionBase = (struct ExpansionBase *)OpenLibrary((STRPTR)"expansion.library", 36)))
        return 20;
    while ((cd = FindConfigDev(cd, 0x6d6e, -1)))
        if (cd->cd_Rom.er_Product == 3 || cd->cd_Rom.er_Product == 4)
            break;
    if (!cd) {
        printf("vrampeek: no ZZ9000\n");
        CloseLibrary((struct Library *)ExpansionBase);
        return 10;
    }
    printf("vrampeek: board at %lx, size %lx\n", (unsigned long)cd->cd_BoardAddr, (unsigned long)cd->cd_BoardSize);
    for (i = 0; i < sizeof(offs) / sizeof(offs[0]); i++) {
        p = (volatile UWORD *)((UBYTE *)cd->cd_BoardAddr + 0x10000 + offs[i]);
        was = *p;
        *p = 0x5a3c;
        got = *p;
        *p = 0xc3a5;
        printf("  +%07lx: wrote 5a3c read %04x, wrote c3a5 read %04x  %s\n", (unsigned long)offs[i], got, *p,
               got == 0x5a3c && *p == 0xc3a5 ? "mapped" : "NOT reached");
        *p = was;
    }
    CloseLibrary((struct Library *)ExpansionBase);
    return 0;
}
