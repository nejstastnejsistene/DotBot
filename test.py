if __name__ == '__main__':
    from dotbot.screenreader import *
    from dotbot.ai import *
    from dotbot import emu
    import time
    filename = 'screenshot.png'
    for i in range(30):
        emu.screenshot('5554:DotBot', filename)
        info = read_game_screen(filename)
        w, path = smart_path(info.colors)
        print w, path
        emu.play_path(info, path)
        time.sleep(2)
    time.sleep(10)
    emu.screenshot('5554:DotBot', filename)
    #emu.click(read_scores_screen(filename))
