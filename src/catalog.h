/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Laine Jones */
/* catalog.h - a tool's texts: S(ID, "English") gives the installed
 * translation or the English. See catalog.c and tools/mkcatalog.py. */
#ifndef PRISM_CATALOG_H
#define PRISM_CATALOG_H

#include <exec/types.h>

#define CAT_NAME    "PrismRTG.catalog"
#define CAT_VERSION 1                   /* bumped when an ID's meaning changes */

void        cat_open(void);
void        cat_close(void);
const char *cat_str(ULONG id, const char *builtin);
const char *cat_language(void);        /* the open catalog's language, or NULL (English built in) */

#define S(id, text) cat_str(id, text)

#include "catalog_ids.h"                /* written by tools/mkcatalog.py --ids */

#endif
