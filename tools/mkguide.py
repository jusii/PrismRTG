#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Laine Jones
"""mkguide.py [dist/ReadMe] [PrismRTG.guide] : the package ReadMe as an AmigaGuide document.

The ReadMe's layout is the structure: a flush-left line followed by indented lines is a section
(its own node, linked from the contents at the top), the lines before the first section are the
introduction, and a run of '=' or '-' under a line is a title. The text is kept as it is - the
ReadMe stays the plain-text original - with AmigaGuide's '@' and '\\' escaped."""
import os, re, sys

src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'dist', 'ReadMe')
out = sys.argv[2] if len(sys.argv) > 2 else 'PrismRTG.guide'

lines = open(src, encoding='latin-1').read().replace('\r', '').split('\n')


def esc(s):
    return s.replace('\\', '\\\\').replace('@', '\\@')


def node_name(title):
    return re.sub(r'[^A-Za-z0-9]+', '_', title).strip('_')[:30] or 'section'


# split: intro, then sections (heading, body lines)
intro, sections, cur = [], [], None
i = 0
while i < len(lines):
    l = lines[i]
    nxt = lines[i + 1] if i + 1 < len(lines) else ''
    is_heading = (l and not l[0].isspace() and not re.match(r'^[=-]+$', l) and
                  (nxt.startswith('  ') or re.match(r'^[=-]+$', nxt)) and len(l) < 70 and
                  not l.endswith(('.', ':', ',')))
    if is_heading and i > 0:
        cur = (l.strip(), [])
        sections.append(cur)
        if re.match(r'^[=-]+$', nxt):
            i += 1
    elif cur is None:
        intro.append(l)
    else:
        cur[1].append(l)
    i += 1

title = intro[0].strip() if intro else 'PrismRTG'
with open(out, 'w', encoding='latin-1', newline='\n') as f:
    f.write('@database "%s"\n@$VER: PrismRTG.guide (from the ReadMe)\n@author "Laine Jones"\n@wordwrap\n\n' % esc(title))
    f.write('@node Main "%s"\n' % esc(title))
    for l in intro:
        if re.match(r'^[=-]+$', l):
            continue
        f.write(esc(l) + '\n')
    f.write('\n')
    for name, body in sections:
        f.write('  @{" %s " link %s}\n' % (esc(name), node_name(name)))
    f.write('@endnode\n\n')
    for name, body in sections:
        f.write('@node %s "%s"\n@toc Main\n' % (node_name(name), esc(name)))
        f.write('%s\n\n' % esc(name))
        while body and not body[-1].strip():
            body.pop()
        for l in body:
            f.write(esc(l) + '\n')
        f.write('@endnode\n\n')
print('%s: %d sections' % (out, len(sections)))
