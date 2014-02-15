from dotbot import emu

def test(game_screen_info):
    emu.click(game_screen_info.shrinker)
    emu.click(game_screen_info.coords[0][0])

def find_simple_path(colors):
    for r in range(6):
        for c in range(5):
            if colors[r][c] == colors[r][c+1]:
                return (r, c), (r, c+1)
    for c in range(6):
        for r in range(5):
            if colors[r][c] == colors[r+1][c]:
                return (r, c), (r+1, c)

def path_to_pixels(path, coords):
    return [coords[r][c] for r, c in path]

def test2(game_screen_info):
    path = find_simple_path(game_screen_info.colors)
    print path, path_to_pixels(path, game_screen_info.coords)
    emu.swipe(path_to_pixels(path, game_screen_info.coords))

if __name__ == '__main__':
    from dotbot.screenreader import read_game_screen
    from commands import getoutput
    getoutput('import -window 5554:DotBot screenshot.png')
    test2(read_game_screen('screenshot.png'))
