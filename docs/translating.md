# Translating PrismRTG's tools

PrismPrefs and PrismSetup take their texts from a locale.library catalog
when one is installed for the user's language, and show English otherwise.
The messages PrismD prints to its log stay English: they are for bug
reports.

## For a translator

1. Start from `catalogs/PrismRTG.cd`: every text with its ID, in the
   CatComp style most Amiga translators know. The numbers never change.
2. Write `catalogs/<language>.ct` (`deutsch.ct`, `français.ct`, ... the
   name locale.library uses). The header gives the language and a date,
   then each ID on a line of its own, the translation on the next, and a
   `;` line:

       ## version 12.10.26
       ## language deutsch
       ID_USE_BLITTER
       Blitter benutzen
       ;

   Keep any `%s`, `%ld`, `%.18s` in a text, in the same order; a `\n` is a
   line break in a requester.
3. Check it: `python3 tools/mkcatalog.py --check catalogs/<language>.ct`
   lists what is missing. A text left out stays English, so a partial
   translation is fine to send.
4. Send it as a pull request or an issue; it ships in the next release
   (`tools/mkpkg.sh` compiles every `.ct` into
   `Catalogs/<language>/PrismRTG.catalog`, and the installer copies them
   to `LOCALE:Catalogs`).

## Trying it

`python3 tools/mkcatalog.py --build catalogs/<language>.ct PrismRTG.catalog`
makes the catalog; put it in `LOCALE:Catalogs/<language>/` on the Amiga.
The Locale preferences choose the language; `SetEnv PRISMLANG <language>`
overrides them for PrismRTG's tools alone (and `UnSetEnv PRISMLANG` ends
that), which is also how to try a translation on an English system.

## For the sources

A text is written as `S(ID_NAME, "English")` (`src/catalog.h`); the build
runs `tools/mkcatalog.py --ids`, which numbers the IDs, writes
`src/catalog_ids.h` and refreshes the `.cd`. Give a new text an ID that
names it; the same text may share an ID. `CAT_VERSION` in catalog.h is
bumped only when an ID's meaning changes, which invalidates old catalogs.
