colors =
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'

falling_dots_duration = 250
segment_animation_duration = 100
finished_path_delay = 500

class Demo

    constructor: (@root) ->
        @queue = []
        @ws = new WebSocket 'ws://localhost:5000', 'dotbot-stream'
        @ws.onopen = -> console.log 'socket opened'
        @ws.onclose = -> console.log 'socket closed'
        @ws.onmessage = (msg) =>
            data = JSON.parse msg.data
            data.grid = ((colors[color] for color in row.split '') for row in data.grid)
            if !data.path?
                @dots = new Dots root, data.grid, @update.bind(@)
            else
                @queue.push data

    update: ->
        if (data = @queue.shift())?
            setTimeout =>
                @dots.draw_path data.path, data.grid, @update.bind(@)
            , 100
        else
            setTimeout @update.bind(@), 50


class Dots

    # Create a dots board in a root element. Calling this calls the dots of
    # the given grid to fall into place.
    constructor: (@root, @grid, next) ->
        @drop_dots next

    # Drops all dots into place, creating new dots as needed. Dots that have been shrunk
    # should be removed from the DOM prior to calling this method. The colors for the new
    # dots falling in is determined by looking at @grid.
    drop_dots: (next) ->
        for c in [0..5]
            # Drop the dots into place, from the bottom up.
            next_row_to_fill = 5
            for r in [5..0]
                if (dot = @get_dot r, c)?
                    @move_dot dot, "row#{r}", "row#{next_row_to_fill--}"
            # Fill in the new dots.
            if next_row_to_fill >= 0
                for r in [0..next_row_to_fill]
                    hidden_row = 5 - next_row_to_fill + r
                    @new_dot hidden_row, r, c
        # Wait long enough for the dots to finish falling before continuing.
        setTimeout next, falling_dots_duration

    # Create a new dot in the DOM. The dot is created in a "hidden row" above the
    # visibile grid, and is then moved to it's proper place via a nice CSS transition
    # that animates it falling into place.
    new_dot: (hr, r, c) ->
        dot = document.createElement 'div'
        dot.className = "dot hidden-row#{hr} col#{c} #{@grid[r][c]}"
        @root.appendChild dot
        @move_dot dot, "hidden-row#{hr}", "row#{r}"

    # Retrieve a dot from the DOM based on its row and column.
    get_dot: (r, c) ->
        @root.getElementsByClassName("dot row#{r} col#{c}")[0]

    # Move a dot from one row to another. This triggers a CSS transition
    # that animates it falling into place.
    move_dot: (dot, old_row_class, new_row_class) ->
        # setTimeout with no delay so that if the element was just created, the
        # runtime will have a moment to create the element before changing the
        # classes so the transition triggers.
        setTimeout ->
            dot.className = dot.className.replace old_row_class, new_row_class

    # Draw a path through the dots.
    draw_path: (path, new_grid, next) ->
        # Mark all of the dots in the path so they can be removed later.
        for [r, c] in path
            @get_dot(r, c).className += ' marked'
        # Recursively draw each path segment.
        do draw_next_segment = (path) =>
            # If there are at least two dots left, draw the next segment.
            if path.length > 1
                @new_path_segment path[0], path[1]
                # Schedule the next segment to be drawn once the animation finishes.
                setTimeout draw_next_segment.bind(@, path.slice 1), segment_animation_duration
            # When the last dot is reached, pause for a while and then shrink the dots.
            else if path.length == 1
                setTimeout @shrink_path.bind(@, new_grid, next), finished_path_delay

    # Create a path segment between two points. The segment draws itself with CSS animations.
    new_path_segment: ([r1, c1], [r2, c2]) ->
        segment = document.createElement 'div'
        segment.className = "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2} #{@grid[r1][c1]}"
        @root.appendChild segment

    # Remove a path and the dots that were affected, then fill in the new dots.
    shrink_path: (@grid, next) ->
        # Remove all the dots marked by a path, as well as the path.
        elements = @root.getElementsByClassName 'dot marked'
        elements[0].parentNode.removeChild elements[0] while elements[0]
        elements = @root.getElementsByClassName 'path-segment'
        elements[0].parentNode.removeChild elements[0] while elements[0]
        # Drop the new dots into place.
        @drop_dots next

setTimeout ->
    demo = new Demo document.getElementById 'dots'
, 500
