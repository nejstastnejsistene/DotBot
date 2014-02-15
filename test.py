from dotbot import emu

def test(game_screen_info):
    emu.click(game_screen_info.shrinker)
    emu.click(game_screen_info.coords[0][0])


if __name__ == '__main__':
    from dotbot.screenreader import read_game_screen
    from commands import getoutput
    getoutput('import -window 5554:DotBot screenshot.png')
    test(read_game_screen('screenshot.png'))
