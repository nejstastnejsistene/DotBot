import array
import dotbot

path = (0, 0), \
       (0, 1), \
       (0, 2), \
       (0, 3), \
       (0, 4), \
       (1, 4), \
       (2, 4), \
       (3, 4), \
       (4, 4), \
       (4, 3), \
       (4, 2), \
       (3, 2), \
       (2, 2), \
       (2, 1), \
       (2, 0), \
       (1, 0), \
       (0, 0), 
path = dotbot.path_from_py(path)
dots = dotbot.get_encircled_dots(path)
print dotbot.path_to_py(dots)
