#!/bin/sh
# tests/catalog.sh : the translation pipeline on the host - the IDs match the sources, the test
# translation compiles to a well-formed IFF CTLG, and every ID it names exists in the .cd.
set -e
cd "$(dirname "$0")/.."
python3 tools/mkcatalog.py --ids tools/prismprefs.c tools/prismsetup.c > /dev/null
grep -q '^ID_USE_BLITTER ' catalogs/PrismRTG.cd || { echo "catalog: ID_USE_BLITTER missing from the .cd"; exit 1; }
n=$(grep -c '^ID_' catalogs/PrismRTG.cd); h=$(grep -c '^#define ID_' src/catalog_ids.h)
[ "$n" = "$h" ] || { echo "catalog: $n texts in the .cd, $h in catalog_ids.h"; exit 1; }
python3 tools/mkcatalog.py --build tests/catalog-test.ct /tmp/PrismRTG-test.catalog > /dev/null
python3 - <<'EOF'
import struct
d = open('/tmp/PrismRTG-test.catalog', 'rb').read()
assert d[:4] == b'FORM' and d[8:12] == b'CTLG', 'not an IFF CTLG'
assert struct.unpack('>I', d[4:8])[0] == len(d) - 8, 'FORM size'
p, chunks, strs = 12, [], 0
while p < len(d):
    tag, size = d[p:p+4], struct.unpack('>I', d[p+4:p+8])[0]
    body = d[p+8:p+8+size]
    chunks.append(tag)
    if tag == b'STRS':
        q = 0
        while q < len(body):
            n, l = struct.unpack('>II', body[q:q+8]); q += 8 + ((l + 3) & ~3); strs += 1
    p += 8 + size + (size & 1)
assert chunks == [b'FVER', b'LANG', b'CSET', b'STRS'], chunks
assert strs == 6, strs
EOF
echo "catalog test passed ($n texts, test catalog of 6)"
