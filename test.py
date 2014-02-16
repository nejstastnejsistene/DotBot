import random
length_of_game = 35

if __name__ == '__main__':
    from dotbot.ai import *
    import dotbot.fakedots
    colors = [[random.randrange(6) for i in range(6)] for r in range(6)]
    #print colors
    for i in range(length_of_game):
        w, path = smart_path(colors) #
        #print w, path
        colors = dotbot.fakedots.play_path(colors, path)
        #print "move made"

    print dotbot.fakedots.totes_score

 
