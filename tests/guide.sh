#!/bin/sh
# tests/guide.sh : the ReadMe converts to an AmigaGuide document whose contents links all have a
# node, with every ReadMe section found (a heading the converter misses would vanish from the guide).
set -e
cd "$(dirname "$0")/.."
python3 tools/mkguide.py dist/ReadMe /tmp/PrismRTG-test.guide > /dev/null
python3 - <<'EOF'
import re
g = open('/tmp/PrismRTG-test.guide', encoding='latin-1').read()
nodes = set(re.findall(r'^@node (\S+)', g, re.M))
links = re.findall(r'link (\S+)\}', g)
missing = [l for l in links if l not in nodes]
assert not missing, 'links without a node: %s' % missing
assert len(nodes) >= 15, 'only %d nodes' % len(nodes)
assert g.startswith('@database'), 'no @database header'
# every flush-left ReadMe line that heads an indented block is a node
r = open('dist/ReadMe', encoding='latin-1').read().replace('\r', '').split('\n')
heads = [l.strip() for i, l in enumerate(r[1:], 1) if l and not l[0].isspace() and not re.match(r'^[=-]+$', l)
         and i + 1 < len(r) and (r[i + 1].startswith('  ') or re.match(r'^[=-]+$', r[i + 1])) and len(l) < 70
         and not l.endswith(('.', ':', ','))]
titles = set(re.findall(r'^@node \S+ "(.*)"', g, re.M))
lost = [h for h in heads if h.replace('\\@', '@') not in {t.replace('\\@', '@') for t in titles}]
assert not lost, 'sections not in the guide: %s' % lost
print('guide test passed (%d nodes, %d links)' % (len(nodes), len(links)))
EOF
