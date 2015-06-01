conf =
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

  # The delay between the previous board finishing animating and beginning
  # to draw the next move.
  drawPathDelay: 100

  # How long it takes the dots to fall into place.
  fallingDotsDuration: 250

  # How long a path segment takes to finish its animation.
  segmentAnimationDuration: 100

  # How long a completed path is shown before it is released and the dots
  # are cleared.
  clearPathDelay: 500


colors =
  R: 'red'
  Y: 'yellow'
  G: 'green'
  B: 'blue'
  V: 'violet'


class Demo

  constructor: ->
    @root = document.getElementById conf.rootId
    @ws = new WebSocket conf.websocketUrl, conf.websocketProtocol
    @ws.onopen = @onopen.bind @
    @ws.onclose = @onclose.bind @
    @ws.onmessage = @onmessage.bind @
    @queue = []

  onopen: ->
    @running = true
    @ws.send 'next'
    @intervalId = setInterval =>
        @ws.send 'next' if @running and @queue.length < conf.targetQueueSize
    , conf.moveRequestInterval

  onclose: ->
    @running = false
    clearInterval @intervalId
    console.log 'socket closed'

  onmessage: (msg) ->
    data = JSON.parse msg.data
    data.grid = ((colors[c] for c in row.split '') for row in data.grid)
    if not data.path?
      @dots = new Dots @root, data.grid, @update.bind(@)
    else
      @queue.push data

  update: ->
    if (data = @queue.shift())?
      setTimeout =>
        @dots.drawPath data.path, data.grid, @update.bind(@)
      , conf.drawPathDelay
    else if @running
      setTimeout @update.bind(@), conf.pollingInterval


class Dots

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
    # Wait long enough for the dots to finish falling before continuing.
    setTimeout next, conf.fallingDotsDuration

  # Create a new dot in the DOM. The dot is created in a "hidden row" above the
  # visibile grid, and is then moved to it's proper place via a nice CSS
  # transition that animates it falling into place.
  newDot: (hr, r, c) ->
    dot = document.createElement 'div'
    dot.className = "dot hidden-row#{hr} col#{c} #{@grid[r][c]}"
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
    setTimeout ->
      dot.className = dot.className.replace oldRowClass, newRowClass

  # Draw a path through the dots.
  drawPath: (path, newGrid, next) ->
    # Mark all of the dots in the path so they can be removed later.
    for [r, c] in path
      @getDot(r, c).className += ' marked'
    # Recursively draw each path segment.
    do drawNextSegment = (path) =>
      # If there are at least two dots left, draw the next segment.
      if path.length > 1
        @newPathSegment path[0], path[1]
        # Schedule the rest of the path to be drawn once the animation finishes.
        setTimeout ->
          drawNextSegment(path.slice 1)
        , conf.segmentAnimationDuration
      # On the last dot, pause for a while and then shrink the dots.
      else if path.length is 1
        setTimeout @clearPath.bind(@, newGrid, next), conf.clearPathDelay

  # Create a path segment between two points. It will animate itself via CSS.
  newPathSegment: ([r1, c1], [r2, c2]) ->
    segment = document.createElement 'div'
    segment.className =
      "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2} #{@grid[r1][c1]}"
    @root.appendChild segment

  # Remove a path and the dots that were affected, then fill in the new dots.
  clearPath: (@grid, next) ->
    # Remove all the dots marked by a path, as well as the path.
    elements = @root.getElementsByClassName 'dot marked'
    elements[0].parentNode.removeChild elements[0] while elements[0]
    elements = @root.getElementsByClassName 'path-segment'
    elements[0].parentNode.removeChild elements[0] while elements[0]
    # Drop the new dots into place.
    @dropDots next


setTimeout ->
  new Demo
, conf.initialDelay
