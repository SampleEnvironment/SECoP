'''
compile README.txt, containing Issue 1 and a linkable list of Issues with their state
'''

from glob import glob
import traceback
import re
import os
from urllib import quote
import io

STATES = {'closed': '\\', 'under discussion':'d', 'unspecified':'u', 'proposed':'p'}
filepat  = re.compile(r'(\d\d\d) (.*).rst')
titlepat = re.compile(r'SECoP Issue (\d*): ([^(]*)(?:\((.*)\))?$')
propername = re.compile(r'[^\w \-\_]+')
issueslist = []
linkdict = {}
files = {}
stdlabels = {}

for filename in sorted(glob('*.rst')):
    try:
        num, title = filepat.match(filename).groups(0)
        num = int(num)
    except AttributeError, ValueError:
        # this is probably not an isse file
        continue
    with open(filename, 'r') as fil:
        content = fil.read().split('\n')
    try:
        tnum, ttitle, state = titlepat.match(content[0]).groups(0)
        stitle = " ".join(propername.sub(' ', ttitle).split())
        tnum = int(tnum)
        assert tnum == num
        assert content[1].startswith("===")
    except:
        print('non standard title:\n%s\n%s' % tuple(content[0:1]))
        continue
    ttitle = ttitle.strip()
    newfilename = "%.3d %s.rst" % (num, stitle)
    label = "SECoP Issue %d: %s" % (num, ttitle)
    newtitle = "%s (%s)" % (label, state)
    print newtitle
    issueslist.append("    %s     `%s`_" % (STATES[state], label))
    files[num] = newfilename
    stdlabels[num] = label
    linkdict[num] = ".. _`%s`: %s" % (label, quote(newfilename))
    if newtitle != content[0]:
        print('change title to: %s', newtitle)
    elif newfilename != filename:
        print('change filename to %s' % newfilename)
    else:
        continue
    content[0] = newtitle
    content[1] = '=' * len(newtitle)
    os.remove(filename)
    with open(newfilename, 'w') as fil:
        fil.write('\n'.join(content))
    

def find_links_labels(link):
    global label
    print link.group(1)
    return link.group(1)

def update_links(filename, files, stdlabels):
    with io.open(filename, encoding='utf-8') as fil:
        content = fil.read()
    pat = re.compile(r'`(SECoP Issue \d+:[^\n`]*)`_')
    numpat = re.compile(r'SECoP Issue (\d+):[^\n`]*')
    links = {}
    labels = {}
    for label in re.findall(pat, content):
        num = int(numpat.match(label).group(1))
        if not num in links:
            links[num] = set()
        links[num].add(label)
    if not links:
        return
    link_list = []
    for num, labels in sorted(links.items()):
        link = []
        for label in labels:
            if label.lower() != stdlabels[num].lower():
                print('WARNING: labels do not match:')
                print('   "%s"' % label)
                print('   "%s"' % stdlabels[num])
            link.append('.. _`%s`:' % label)
        link_list.append('\n'.join(link) + ' issues/%s' % quote(files[num]))
    marker = '.. DO NOT TOUCH --- %s links are automatically updated by issue/makeissuelist.py\n'
    cleaned = re.sub(r'.. _`SECoP Issue \d+: .*`: .*\n', '', content)
    parts = re.split(marker % r'\w*', cleaned)
    while len(parts) < 3:
        parts.append('')
    newcontent = parts[0] + marker % 'following' + '\n'.join(link_list + ['']) + marker % 'above' + parts[2]
    if newcontent != content:
        print('update links in %s' %filename)
        with io.open(filename, 'w', encoding='utf-8') as fil:
            fil.write(newcontent)



with open('001 About SECoP Issues.rst', 'r') as fil:
    text = fil.read().split('\n')
text=['','']

text[0] = 'SECoP Issues'
text[1] = '============'

header = '    ===== ======='
with open('README.rst', 'w') as fil:
    fil.write("\n".join(text + ['Issues List\n===========', '\n.. table::\n', header] + issueslist
                        + [header, ''] + list(linkdict.values())))

for filename in glob('../*.rst'):
    update_links(filename, files, stdlabels)

for filename in glob('*.rst'):
    if filename != 'README.rst':
        update_links(filename, files, stdlabels)
