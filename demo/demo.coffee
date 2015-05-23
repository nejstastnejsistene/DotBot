colors =
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'
rand_color = -> colors[Object.keys(colors)[Math.floor(Math.random()*5)]]

class Dots
    constructor: (@grid) ->
        @drop_dots()
    drop_dots: ->
        for c in [0..5]
            row = 5
            for r in [5..0]
                dot = $("#dots .row#{r}.col#{c}")
                if dot.length
                    if r != row
                        do (dot, row, r) -> setTimeout ->
                            dot.addClass("row#{row}").removeClass("row#{r}")
                    row--
            if row >= 0
                for r in [0..row]
                    hr = 5-row+r
                    $("#dots").append "<div class='dot hidden-row#{hr} col#{c} #{@grid[r][c]}'/>"
                    do (hr, r, c) -> setTimeout ->
                        $("#dots .hidden-row#{hr}.col#{c}").addClass("row#{r}").removeClass("hidden-row#{hr}")
    shrink_marked_dots: ->
        $('#dots .dot.marked').remove()
        @drop_dots()

dots = undefined

#setInterval ->
#    dots.grid = (rand_color() for r in [0..5] for c in [0..5])
#    x = []
#    while x.length == 0
#        x = $("#dots .dot.#{rand_color()}")
#    x.addClass 'marked'
#    dots.shrink_marked_dots()
#, 1000

w = new WebSocket 'ws://localhost:5000', 'dotbot-stream'
w.onopen = ->
    w.send 'start'
w.onmessage = (msg) ->
    {grid, path} = JSON.parse(msg.data)
    grid = ((colors[color] for color in row.split '') for row in grid)
    setTimeout ->
        if !path?
            setTimeout ->
                dots = new Dots(grid)
            , 500
        else
            dots.grid = grid
            for [r, c] in path
                $("#dots .dot.row#{r}.col#{c}").addClass 'marked'
            dots.shrink_marked_dots()
w.onclose = -> console.log 'socket closed'

$('#start-stop').on 'click', ->
    if $(@).html() == 'Start'
        w.send 'start'
        $(@).html 'Stop'
    else
        w.send 'stop'
        $(@).html 'Start'
$('#reset').on 'click', ->
    $('#dots .dot').remove()
    w.send 'reset'
