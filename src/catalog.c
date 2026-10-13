/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/*
 * catalog.c - the tools' texts through locale.library, when a translation
 * is installed (LOCALE:Catalogs/<language>/PrismRTG.catalog), and the
 * built-in English otherwise. Every text in a tool is written as
 * S(ID_NAME, "English text"); tools/mkcatalog.py reads those, numbers the
 * IDs (catalog_ids.h), writes the .cd for translators and compiles a .ct
 * into a .catalog. A tool calls cat_open() first and cat_close() last;
 * without locale.library (OS 2.0) everything stays English.
 */
#include <exec/types.h>
#include <libraries/locale.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include "catalog.h"

/* libnix opens a library itself at start-up when the program references
 * its base (LocaleBase here), so the base may already be set when
 * cat_open runs: it opens the library only when there is none, and closes
 * only what it opened. */
struct LocaleBase *LocaleBase;
static struct Catalog *cat;
static BOOL opened, tried;

void cat_open(void)
{
    if (tried)
        return;
    tried = TRUE;
    if (!LocaleBase) {
        LocaleBase = (struct LocaleBase *)OpenLibrary((STRPTR)"locale.library", 38);
        opened = LocaleBase != NULL;
    }
    if (LocaleBase) {
        static char lang[32];
        struct TagItem tags[4];
        UWORD n = 0;
        tags[n].ti_Tag = OC_BuiltInLanguage; tags[n++].ti_Data = (ULONG)"english";
        tags[n].ti_Tag = OC_Version;         tags[n++].ti_Data = CAT_VERSION;
        /* ENV:PRISMLANG names a language ahead of the Locale preferences:
         * for trying a translation, and for a user who wants the tools in
         * another language than the system */
        lang[0] = 0;
        if (GetVar((STRPTR)"PRISMLANG", (STRPTR)lang, sizeof(lang), 0) > 0) {
            char *e = lang;              /* "echo >ENV:PRISMLANG deutsch" leaves a newline */
            while (*e && *e != 10 && *e != 13 && *e != 32)   /* to the first newline, CR or space */
                e++;
            *e = 0;
        }
        if (lang[0]) {
            tags[n].ti_Tag = OC_Language;    tags[n++].ti_Data = (ULONG)lang;
        }
        tags[n].ti_Tag = TAG_DONE;           tags[n].ti_Data = 0;
        cat = OpenCatalogA(NULL, (STRPTR)CAT_NAME, tags);
    }
}

void cat_close(void)
{
    if (cat) {
        CloseCatalog(cat);
        cat = NULL;
    }
    if (opened) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
        opened = FALSE;
    }
    tried = FALSE;
}

const char *cat_language(void)
{
    return cat ? (const char *)cat->cat_Language : NULL;
}

const char *cat_str(ULONG id, const char *builtin)
{
    if (!cat)
        return builtin;
    return (const char *)GetCatalogStr(cat, id, (STRPTR)builtin);
}
