colors =
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'
rand_color = -> colors[Object.keys(colors)[Math.floor(Math.random()*5)]]


class Demo

    constructor: ->
        @queue = []
        @ws = new WebSocket 'ws://localhost:5000', 'dotbot-stream'
        @ws.onopen = -> @send 'start'
        @ws.onclose = -> console.log 'socket closed'
        @ws.onmessage = (msg) =>
            data =  JSON.parse msg.data
            data.grid = ((colors[color] for color in row.split '') for row in data.grid)
            if !data.path?
                @dots = new Dots data.grid
                @start()
            else
                @queue.push data

    update: ->
        data = @queue.shift()
        if data?
            {grid, path} = data
            @dots.draw_path path
            setTimeout @dots.shrink_marked_dots.bind(@dots, grid), 500

    start: ->
        @interval_id = setInterval @update.bind(@), 1000

    stop: ->
        if @interval_id?
            clearInterval @interval_id


class Dots

    context: $('#dots')

    constructor: (@grid) ->
        @$ = (selector) -> $(selector, @context)
        @drop_dots()

    drop_dots: ->
        for c in [0..5]
            row = 5
            for r in [5..0]
                dot = @$(".row#{r}.col#{c}")
                if dot.length
                    if r != row
                        do (dot, row, r) -> setTimeout ->
                            dot.addClass("row#{row}").removeClass("row#{r}")
                    row--
            if row >= 0
                for r in [0..row]
                    hr = 5-row+r
                    @context.append "<div class='dot hidden-row#{hr} col#{c} #{@grid[r][c]}'/>"
                    do (hr, r, c) -> setTimeout ->
                        @$(".hidden-row#{hr}.col#{c}").addClass("row#{r}").removeClass("hidden-row#{hr}")

    draw_path: (path) ->
        [r1, c1] = path[0]
        color = @grid[r1][c1]
        for [r2, c2] in path.slice(1)
            @context.append @new_path(r1, c1, r2, c2, color)
            [r1, c1] = [r2, c2]
        for [r, c] in path
            @$(".dot.row#{r}.col#{c}").addClass 'marked'

    new_path: (r1, c1, r2, c2, color) ->
        if r1 > r2 or c1 > c2
            [r1, c1, r2, c2] = [r2, c2, r1, c1]
        "<div class='path from-row#{r1} from-col#{c1} to-row#{r2} to-col#{c2} #{color}'/>"

    shrink_marked_dots: (@grid) ->
        @$('.path').remove()
        @$('.dot.marked').remove()
        @drop_dots()


demo = new Demo()

$('#start-stop').on 'click', ->
    if $(@).html() == 'Start'
        demo.ws.send 'start'
        $(@).html 'Stop'
    else
        demo.stop()
        demo.ws.send 'stop'
        $(@).html 'Start'
$('#reset').on 'click', ->
    $('#dots .path').remove()
    $('#dots .dot').remove()
    demo.ws.send 'reset'
