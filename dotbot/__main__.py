import dotbot

screen = dotbot.readscreen()
if isinstance(screen, dotbot.screenreader.GameScreen):
    path = list(dotbot.DotBot(screen))
    script = dotbot.reran.RERANScriptMT()
    if len(path) == 1:
        script.click(*path[0])
        script.click(*path[0])
    else:
        script.gesture(path)
    dotbot.sendevents(script)

