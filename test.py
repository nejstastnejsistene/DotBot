import random
length_of_game = 35

if __name__ == '__main__':
    from dotbot.ai import *
    import dotbot.fakedots
    colors = [[random.randrange(5) for i in range(6)] for r in range(6)]
    #print colors
    for j in range(25):
        for i in range(length_of_game):
            w, path = smart_path(colors) #
            #print w, path
            colors = dotbot.fakedots.play_path(colors, path)
            #print "move made"
            if dotbot.fakedots.totes_score < 5 or (i>5 and dotbot.fakedots.totes_score/i < 10 ): 
                dotbot.fakedots.totes_score = 0
                break

        print dotbot.fakedots.totes_score
        dotbot.fakedots.totes_score = 0

 
