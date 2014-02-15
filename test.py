if __name__ == '__main__':
    from dotbot.screenreader import *
    from dotbot.ai import *
    from dotbot import emu
    import time
    filename = 'screenshot.png'
    while True:
        for i in range(30):
            emu.screenshot('5554:DotBot', filename)
            info = read_game_screen(filename)
            path = lj_algorithm(info.colors)
            emu.play_path(info, path)
            time.sleep(1)
        time.sleep(10)
        emu.screenshot('5554:DotBot', filename)
        emu.click(read_scores_screen(filename))
        time.sleep(1)
