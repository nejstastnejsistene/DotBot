import random
length_of_game = 35

if __name__ == '__main__':
    from dotbot.ai import *
    import dotbot.fakedots
    colors = [[random.randrange(5) for i in range(6)] for r in range(6)]
    #print colors
    complete = 0
    while complete < 10:
        print "completed:", complete
        for i in range(length_of_game):
            w, path = smart_path(colors) #
            #print w, path
            colors = dotbot.fakedots.play_path(colors, path)
            print "move made"
            if dotbot.fakedots.totes_score < 5 or (i>5 and dotbot.fakedots.totes_score/i < 10 ): 
                dotbot.fakedots.totes_score = 0
                print "quitting early"
                complete -= 1
                break

        print dotbot.fakedots.totes_score
        dotbot.fakedots.totes_score = 0
        complete += 1

 
