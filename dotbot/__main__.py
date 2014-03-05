import dotbot
from commands import getoutput

print getoutput('adb shell screencap /data/local/DotBot/screenshot.raw')
x, y = getoutput('adb shell /data/local/DotBot/readscreen').split('\n', 1)
path = list(dotbot.DotBot(x, y))
script = dotbot.reran.RERANScriptMT()
if len(path) == 1:
    script.click(*path[0])
    script.click(*path[0])
else:
    script.gesture(dotbot.reran.interpolate(path))
dotbot.sendevents(script)

