colors = ['red', 'yellow', 'green', 'blue', 'violet']
rand_color = -> colors[Math.floor(Math.random()*5)]

new_dot = (r, c, color) ->
    "<div class='dot hidden-row#{5-r} col#{c} #{color}'/>"
    
drop_dots = ->
    for r in [0..5]
        do (r) -> setTimeout ->
            $(".hidden-row#{5-r}").addClass("row#{r}").removeClass("hidden-row#{5-r}")

for r in [0..5]
    for c in [0..5]
        $('#dots').append new_dot(r, c, rand_color())
        
setTimeout drop_dots, 1000
