import dotbot
from commands import getoutput

print getoutput('adb shell screencap /data/local/DotBot/screenshot.raw')
x, y = getoutput('adb shell /data/local/DotBot/readscreen').split('\n', 1)
path = list(dotbot.DotBot(x, y))
asdf = ''
for x, y in path:
    asdf += '{} {}\n'.format(x, y)
with open('path.txt', 'w+') as f:
    f.write(asdf)
print getoutput('adb push path.txt /data/local/DotBot/')
print getoutput('adb shell "/data/local/DotBot/foo < /data/local/DotBot/path.txt"')
'''
script = dotbot.reran.RERANScriptMT()
if len(path) == 1:
    script.click(*path[0])
    script.click(*path[0])
else:
    script.gesture(dotbot.reran.interpolate(path))
dotbot.sendevents(script)
'''
