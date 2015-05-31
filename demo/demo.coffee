colors =
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'
rand_color = -> colors[Object.keys(colors)[Math.floor(Math.random()*5)]]

falling_dots_transition_ms = 250
segment_transition_ms = 100

class Demo

    constructor: ->
        @queue = []
        @ws = new WebSocket 'ws://localhost:5000', 'dotbot-stream'
        @ws.onopen = -> console.log 'socket opened'
        @ws.onclose = -> console.log 'socket closed'
        @ws.onmessage = (msg) =>
            data =  JSON.parse msg.data
            data.grid = ((colors[color] for color in row.split '') for row in data.grid)
            if !data.path?
                @dots = new Dots data.grid, @update.bind(@)
            else
                @queue.push data

    update: ->
        data = @queue.shift()
        if data?
            {path, grid} = data
            setTimeout =>
                @dots.draw_path path, grid, @update.bind(@)
            , 100
        else
            setTimeout @update.bind(@), 50


class Dots

    context: $('#dots')
    $: (selector) -> $(selector, @context)

    constructor: (@grid, next) ->
        @drop_dots next

    drop_dots: (next) ->
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
        setTimeout next, falling_dots_transition_ms

    draw_path: (path, new_grid, next) ->
        for [r, c] in path
            @$(".dot.row#{r}.col#{c}").addClass 'marked'
        do draw_next_segment = (path) =>
            if path.length > 1
                [r1, c1] = path[0]
                [r2, c2] = path[1]
                @context.append "<div class='path from-#{r1}-#{c1}-to-#{r2}-#{c2} #{@grid[r1][c1]}'/>"
                setTimeout draw_next_segment.bind(@, path.slice(1)), segment_transition_ms
            else if path.length == 1
                setTimeout @shrink_marked_dots.bind(@, new_grid, next), 500

    shrink_marked_dots: (@grid, next) ->
        @$('.path').remove()
        @$('.dot.marked').remove()
        @drop_dots next


setTimeout ->
    demo = new Demo()
, 500
