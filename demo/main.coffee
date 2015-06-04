new class Demo

  # The root element's id.
  rootId: 'dots'

  # WebSocket parameters.
  webSocketUrl: 'ws://localhost:5000'
  webSocketProtocol: 'dotbot-stream'

  # Delay in ms before starting.
  initialDelay: 500

  # How frequently new moves are requested.
  moveRequestInterval: 500

  reconnectDelay: 5000

  # The maximum number of moves that should be queued.
  targetQueueSize: 20

  # How frequently to check an empty queue for moves.
  pollingInterval: 1000

  colors:
    R: 'red'
    Y: 'yellow'
    G: 'green'
    B: 'blue'
    V: 'violet'

  parseGrid: (grid) -> ((@colors[c] for c in row.split '' ) for row in grid)
  fmtGrid: (grid) -> ((c[0].toUpperCase() for c in row).join '' for row in grid)

  constructor: ->
    @root = document.getElementById @rootId
    @queue = []
    setTimeout @createWebSocket.bind(this), @initialDelay

  createWebSocket: ->
    @ws = new WebSocket(@webSocketUrl, @webSocketProtocol)
    @ws.onopen = @onopen.bind this
    @ws.onclose = @onclose.bind this
    @ws.onmessage = @onmessage.bind this

  onopen: (e) ->
    if @latestGrid?
      e.target.send 'setGrid:' + JSON.stringify @fmtGrid(@latestGrid)
    @updateIntervalId = setInterval =>
      e.target.send 'next' if @queue.length < @targetQueueSize
    , @moveRequestInterval

  onclose: ->
    console.log new Date(), 'disconnected'
    clearInterval @updateIntervalId
    setTimeout =>
      console.log new Date(), 'reconnecting...'
      @createWebSocket()
    , @reconnectDelay

  onmessage: (msg) ->
    data = JSON.parse msg.data
    data.grid = @latestGrid = @parseGrid data.grid
    if not data.path?
      @dots = new Dots(@root, data.grid, @update.bind this)
    else
      @queue.push data

  update: ->
    if (data = @queue.shift())?
      @dots.drawPath data.path, data.grid, @update.bind(this)
    else
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

  getDot: (r, c) -> @dots[r][c]
  setDot: (r, c, dot) -> @dots[r][c] = dot
  delDot: (r, c) -> delete @dots[r][c]

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
            dot.row nextRowToFill
            @setDot nextRowToFill, c, dot
            @delDot r, c
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
    dot = new Dot(r, c, @grid[r][c])
    dot.animateFall()
    @setDot r, c, dot
    @root.appendChild dot.element

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
        setTimeout @clearPath.bind(this, newGrid, next), @clearPathDelay

  # Mark the dot as selected. If the dot is already selected (assuming
  # ignoreSelected is false) it will mark every dot of the same color
  # as selected.
  selectDot: (dot, checkForCycle = true) ->
    if checkForCycle and dot.isMarkedForDeletion()
      setTimeout =>
        @root.dataset.color = color = dot.color()
        for r in [0..5]
          for c in [0..5]
            if (dot = @getDot r, c).color() is color
              @selectDot dot, false
      return

    if not dot.isMarkedForDeletion()
      dot.markForDeletion()
      @toBeRemoved ?= []
      @toBeRemoved.push dot

    dot.selection.element.remove() if dot.selection?
    dot.selection = new Dot(dot.row(), dot.col(), dot.color())
    dot.selection.animateSelection()
    @root.appendChild dot.selection.element

  # Create a path segment between two points. It will animate itself via CSS.
  newPathSegment: ([r1, c1], [r2, c2]) ->
    segment = document.createElement 'div'
    segment.className = "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2}"
    segment.dataset.color = @grid[r1][c1]
    @root.appendChild segment

    @toBeRemoved ?= []
    @toBeRemoved.push segment

    segment

  # Remove a path and the dots that were affected, then fill in the new dots.
  clearPath: (newGrid, next) ->
    delete @root.dataset.color if @root.dataset.color?
    for x in @toBeRemoved
      if x instanceof Dot
        @delDot x.row(), x.col()
        x.animateShrinking()
      else
        x.remove()
    @toBeRemoved = []
    setTimeout @dropDots.bind(this, newGrid, next), @dropDotsDelay


class Dot

  constructor: (r, c, color) ->
    @element = document.createElement 'div'
    @element.className = 'dot'
    @row(r)
    @col(c)
    @color(color)
    @element.dot = this

  row: (x) -> @element.dataset.row = x ? @element.dataset.row
  col: (x) -> @element.dataset.col = x ? @element.dataset.col
  color: (x) -> @element.dataset.color = x ? @element.dataset.color

  markForDeletion: -> @element.classList.add 'marked-for-deletion'
  isMarkedForDeletion: -> @element.classList.contains 'marked-for-deletion'

  animateFall: ->
    @element.classList.add 'falling'
    @element.addEventListener 'animationend', (e) ->
      e.target.classList.remove 'falling'

  animateSelection: -> @animateAndRemove 'selecting'
  animateShrinking: -> @animateAndRemove 'shrinking'

  animateAndRemove: (className) ->
    @element.classList.add className
    @element.addEventListener 'animationend', (e) -> e.target.remove()
