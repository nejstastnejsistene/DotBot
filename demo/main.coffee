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
      @ws = new WebSocket(@websocketUrl, @websocketProtocol)
      @ws.onopen = @onopen.bind this
      @ws.onclose = @onclose.bind this
      @ws.onmessage = @onmessage.bind this
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
      @dots = new Dots(@root, data.grid, @update.bind this)
    else
      @queue.push data

  update: ->
    if (data = @queue.shift())?
      @dots.drawPath data.path, data.grid, @update.bind(this)
    else if @running
      setTimeout @update.bind(this), @pollingInterval


class Dots

  # How long it takes the dots to fall into place.
  fallingDotsDuration: 250

  # The delay between the dots falling into place and drawing the next move.
  drawPathDelay: 100

  # How long a completed path is shown before the dots are cleared.
  clearPathDelay: 500

  dropDotsDelay: 150

  # Create a dots board in a root element. Calling this calls the dots of
  # the given grid to fall into place.
  constructor: (@root, @grid, next) ->
    @dots = ([] for r in [0..5])
    @dropDots @grid, next

  # Drops all dots into place, creating new dots as needed. Dots that have
  # been shrunk should be removed from the DOM prior to calling this method.
  # The colors for the new dots falling in is determined by looking at @grid.
  dropDots: (@grid, next) ->
    for c in [0..5]
      # Drop the dots into place, from the bottom up.
      nextRowToFill = 5
      for r in [5..0]
        if (dot = @getDot r, c)?
          if r isnt nextRowToFill
            @setDot nextRowToFill, c, dot
            @delDot r, c
            dot.dataset.row = nextRowToFill
          nextRowToFill -= 1
      # Fill in the new dots.
      if nextRowToFill >= 0
        for r in [0..nextRowToFill]
          @newDot r, c
    setTimeout next, @fallingDotsDuration + @drawPathDelay

  # Create a new dot in the DOM. The dot is created in a "hidden row" above the
  # visibile grid, and is then moved to it's proper place via a nice CSS
  # transition that animates it falling into place.
  newDot: (r, c) ->
    dot = document.createElement 'div'
    dot.className = 'dot'
    dot.dataset.row = r
    dot.dataset.col = c
    dot.dataset.color = @grid[r][c]
    @root.appendChild dot
    @setDot r, c, dot

  getDot: (r, c) -> @dots[r][c]
  setDot: (r, c, dot) -> @dots[r][c] = dot
  delDot: (r, c) -> delete @dots[r][c]

  # Draw a path through the dots.
  drawPath: (path, newGrid, next) ->
    # Recursively draw each path segment.
    do drawNextSegment = (path) =>
      [r, c] = path[0]
      @selectDot @getDot(path[0]...)
      # If there are at least two dots left, draw the next segment.
      if path.length > 1
        segment = @newPathSegment path[0], path[1]
        # Schedule the rest of the path to be drawn once the animation finishes.
        segment.addEventListener 'animationend', ->
          drawNextSegment(path.slice 1)
      # On the last dot, pause for a while and then shrink the dots.
      else if path.length is 1
        setTimeout @clearPath.bind(this, newGrid, next), @clearPathDelay

  # Mark the dot as selected. If the dot is already selected (assuming
  # ignoreSelected is false) it will mark every dot of the same color
  # as selected.
  selectDot: (dot, checkForCycle = true) ->
    if checkForCycle and dot.classList.contains 'marked-for-deletion'
      setTimeout =>
        color = dot.dataset.color
        @root.dataset.color = color
        for r in [0..5]
          for c in [0..5]
            if (dot = @getDot r, c).dataset.color is color
              @selectDot dot, false
      return

    dot.classList.add 'marked-for-deletion'

    anim = document.createElement 'div'
    anim.className = 'dot selecting'
    anim.dataset.row = dot.dataset.row
    anim.dataset.col = dot.dataset.col
    anim.dataset.color = dot.dataset.color
    anim.addEventListener 'animationend', ->
      e.target.parentNode.removeChild e.target
    @root.appendChild anim

  # Create a path segment between two points. It will animate itself via CSS.
  newPathSegment: ([r1, c1], [r2, c2]) ->
    segment = document.createElement 'div'
    segment.className = "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2}"
    segment.dataset.color = @grid[r1][c1]
    @root.appendChild segment
    segment

  # Remove a path and the dots that were affected, then fill in the new dots.
  clearPath: (newGrid, next) ->
    delete @root.dataset.color if @root.dataset.color?
    segments = @root.getElementsByClassName 'path-segment'
    segments[0].parentNode.removeChild segments[0] while segments[0]
    selectedDots = @root.getElementsByClassName 'dot marked-for-deletion'
    for dot in selectedDots
      @delDot dot.dataset.row, dot.dataset.col
      dot.classList.add 'shrinking'
      dot.addEventListener 'animationend', ->
        e.target.parentNode.removeChild e.target
    setTimeout @dropDots.bind(this, newGrid, next), @dropDotsDelay
