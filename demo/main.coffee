new class Demo

  # The root element's id.
  rootId: 'dots'

  # Websocket parameters.
  websocketUrl: 'ws://localhost:5000'
  websocketProtocol: 'dotbot-stream'

  # Delay in ms before starting.
  initialDelay: 500

  # How frequently new moves are requested.
  moveRequestInterval: 500

  # The maximum number of moves that should be queued.
  targetQueueSize: 20

  # How frequently to check an empty queue for moves.
  pollingInterval: 50

  colors:
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'

  constructor: ->
    @root = document.getElementById @rootId
    @queue = []
    setTimeout =>
      @ws = new WebSocket @websocketUrl, @websocketProtocol
      @ws.onopen = @onopen.bind @
      @ws.onclose = @onclose.bind @
      @ws.onmessage = @onmessage.bind @
    , @initialDelay

  onopen: ->
    @running = true
    @updateIntervalId = setInterval =>
      @ws.send 'next' if @running and @queue.length < @targetQueueSize
    , @moveRequestInterval

  onclose: ->
    @running = false
    clearInterval @updateIntervalId
    console.log 'socket closed'

  onmessage: (msg) ->
    data = JSON.parse msg.data
    data.grid = ((@colors[c] for c in row.split '') for row in data.grid)
    if not data.path?
      @dots = new Dots @root, data.grid, @update.bind(@)
    else
      @queue.push data

  update: ->
    if (data = @queue.shift())?
      @dots.drawPath data.path, data.grid, @update.bind(@)
    else if @running
      setTimeout @update.bind(@), @pollingInterval


class Dots

  # How long it takes the dots to fall into place.
  fallingDotsDuration: 250

  # The delay between the dots falling into place and drawing the next move.
  drawPathDelay: 100

  # How long a completed path is shown before the dots are cleared.
  clearPathDelay: 500

  # Create a dots board in a root element. Calling this calls the dots of
  # the given grid to fall into place.
  constructor: (@root, @grid, next) ->
    @dropDots next

  # Drops all dots into place, creating new dots as needed. Dots that have
  # been shrunk should be removed from the DOM prior to calling this method.
  # The colors for the new dots falling in is determined by looking at @grid.
  dropDots: (next) ->
    for c in [0..5]
      # Drop the dots into place, from the bottom up.
      nextRowToFill = 5
      for r in [5..0]
        if (dot = @getDot r, c)?
          @moveDot dot, "row#{r}", "row#{nextRowToFill--}"
      # Fill in the new dots.
      if nextRowToFill >= 0
        for r in [0..nextRowToFill]
          hiddenRow = 5 - nextRowToFill + r
          @newDot hiddenRow, r, c
    setTimeout next, @fallingDotsDuration + @drawPathDelay

  # Create a new dot in the DOM. The dot is created in a "hidden row" above the
  # visibile grid, and is then moved to it's proper place via a nice CSS
  # transition that animates it falling into place.
  newDot: (hr, r, c) ->
    dot = document.createElement 'div'
    dot.className = "dot hidden-row#{hr} col#{c}"
    dot.dataset.color = @grid[r][c]
    @root.appendChild dot
    @moveDot dot, "hidden-row#{hr}", "row#{r}"

  # Retrieve a dot from the DOM based on its row and column.
  getDot: (r, c) ->
    @root.getElementsByClassName("dot row#{r} col#{c}")[0]

  # Move a dot from one row to another. This triggers a CSS transition
  # that animates it falling into place.
  moveDot: (dot, oldRowClass, newRowClass) ->
    # setTimeout with no delay so that if the element was just created, the
    # runtime will have a moment to create the element before changing the
    # classes so the transition triggers.
    setTimeout =>
      dot.classList.remove oldRowClass
      dot.classList.add newRowClass
    , 100

  # Draw a path through the dots.
  drawPath: (path, newGrid, next) ->
    # Recursively draw each path segment.
    do drawNextSegment = (path) =>
      @selectDot @getDot(path[0]...)
      # If there are at least two dots left, draw the next segment.
      if path.length > 1
        segment = @newPathSegment path[0], path[1]
        # Schedule the rest of the path to be drawn once the animation finishes.
        segment.addEventListener 'animationend', ->
          drawNextSegment(path.slice 1)
      # On the last dot, pause for a while and then shrink the dots.
      else if path.length is 1
        setTimeout @clearPath.bind(@, newGrid, next), @clearPathDelay

  # Mark the dot as selected. If the dot is already selected (assuming
  # ignoreSelected is false) it will mark every dot of the same color
  # as selected.
  selectDot: (dot, checkForCycle=true) ->
    if dot.classList.contains 'selected'
      dot.classList.remove 'selected'
      if checkForCycle
        setTimeout =>
          color = dot.dataset.color
          @root.dataset.color = color
          for r in [0..5]
            for c in [0..5]
              if (dot = @getDot r, c).dataset.color == color
                @selectDot dot, false
        return
    setTimeout =>
      dot.classList.add 'selected'
      anim = document.createElement 'div'
      anim.className = dot.className
      anim.classList.add 'selecting'
      anim.dataset.color = dot.dataset.color
      @root.appendChild anim

  # Create a path segment between two points. It will animate itself via CSS.
  newPathSegment: ([r1, c1], [r2, c2]) ->
    segment = document.createElement 'div'
    segment.className = "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2}"
    segment.dataset.color = @grid[r1][c1]
    @root.appendChild segment
    segment

  # Remove a path and the dots that were affected, then fill in the new dots.
  clearPath: (@grid, next) ->
    delete @root.dataset.color if @root.dataset.color?
    segments = @root.getElementsByClassName 'path-segment'
    segments[0].parentNode.removeChild segments[0] while segments[0]
    selectedDots = @root.getElementsByClassName 'dot selected'
    for dot in selectedDots
      dot.className += ' shrinking'
      dot.addEventListener 'animationend', (e) =>
        if e.animationName == 'shrinking'
          e.target.parentNode.removeChild e.target
          if selectedDots.length is 0
            setTimeout => @dropDots next
