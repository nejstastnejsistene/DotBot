import struct
from PIL import Image

with open('screenshot.raw') as s:
    w, h, f = struct.unpack('III', s.read(12))
    assert f == 1
    img = Image.frombuffer('RGB', (w, h), s.read(), 'raw', 'RGBA', 0, 1)
    img.save('out.png')
