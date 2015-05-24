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
    draw_path: (path) ->
        [r1, c1] = path[0]
        color = dots.grid[r1][c1]
        for [r2, c2] in path.slice(1)
            if r1 < r2 || c1 < c2
                x = "<div class='path from-row#{r1} from-col#{c1} to-row#{r2} to-col#{c2} #{color}'/>"
            else
                x = "<div class='path from-row#{r2} from-col#{c2} to-row#{r1} to-col#{c1} #{color}'/>"
            $('#dots').append x
            [r1, c1] = [r2, c2]
        for [r, c] in path
            $("#dots .dot.row#{r}.col#{c}").addClass 'marked'

    shrink_marked_dots: (@grid) ->
        $('#dots .path').remove()
        $('#dots .dot.marked').remove()
        @drop_dots()

dots = undefined

w = new WebSocket 'ws://localhost:5000', 'dotbot-stream'

w.onopen = ->
    w.send 'start'

w.onmessage = (msg) ->
    {grid, path} = JSON.parse(msg.data)
    grid = ((colors[color] for color in row.split '') for row in grid)
    setTimeout ->
        if !path?
            dots = new Dots grid
        else
            dots.draw_path path
            setTimeout dots.shrink_marked_dots.bind(dots, grid), 500

w.onclose = ->
    console.log 'socket closed'

$('#start-stop').on 'click', ->
    if $(@).html() == 'Start'
        w.send 'start'
        $(@).html 'Stop'
    else
        w.send 'stop'
        $(@).html 'Start'
$('#reset').on 'click', ->
    $('#dots .path').remove()
    $('#dots .dot').remove()
    w.send 'reset'
