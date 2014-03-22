import sys
import struct
from PIL import Image

if len(sys.argv) < 2:
    print >> sys.stderr, 'usage: {} <infile> [<outfile>]'
    sys.exit(1)
with open(sys.argv[1]) as s:
    w, h, f = struct.unpack('III', s.read(12))
    assert f == 1
    img = Image.frombuffer('RGBA', (w, h), s.read(), 'raw', 'RGBA', 0, 1)
    img.save(sys.argv[2] if len(sys.argv) > 2 else 'out.png')
