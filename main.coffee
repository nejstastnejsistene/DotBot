new class Demo

  # The root element's id.
  rootId: 'dots'

  # WebSocket parameters.
  webSocketUrl: 'wss://dotbot.herokuapp.com'
  webSocketProtocol: 'dotbot-stream'

  # Delay in ms before starting.
  initialDelay: 500

  # How frequently new moves are requested.
  moveRequestInterval: 500

  # How long to wait before attempting to reconnect.
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

  # Converting the grid from and to the server's format.
  # Our format: 6x6 array of lower case color names
  # Server format: 1D array where each row is a string of "RYGBV"
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
    # If the @latestGrid is set, then this is reconnection. Let the server
    # know where we left off so we can continue the same game.
    if @latestGrid?
      e.target.send 'setGrid:' + JSON.stringify @fmtGrid(@latestGrid)
    # Request new moves at regular interval, but only if the queue isn't full.
    @updateIntervalId = setInterval =>
      e.target.send 'next' if @queue.length < @targetQueueSize
    , @moveRequestInterval

  onclose: ->
    console.log new Date(), 'disconnected'
    # Stop updating on the old connection, and then reconnect.
    clearInterval @updateIntervalId
    setTimeout =>
      console.log new Date(), 'reconnecting...'
      @createWebSocket()
    , @reconnectDelay

  onmessage: (msg) ->
    data = JSON.parse msg.data
    data.grid = @latestGrid = @parseGrid data.grid
    # The server sends its first message without a path to indicate that
    # it's a new board. If there's no path, then create the initial dots!
    # Otherwise put it in a queue to be dealt with later.
    if data.newGrid
      @dots = new Dots(@root, data.grid, @update.bind this)
    else
      @queue.push data

  update: ->
    # If theres a new move in the queue, play it. Otherwise, try again later.
    if (data = @queue.shift())?
      if data.path?
        @dots.drawPath data.path, data.grid, @update.bind(this)
      if data.shrinkRandom?
        @dots.shrinkRandom data.shrinkRandom, data.grid, @update.bind(this)
    else
      setTimeout @update.bind(this), @pollingInterval


class Dots

  # How long it takes the dots to fall into place.
  fallingDotsDuration: 250

  # The delay between the dots falling into place and drawing the next move.
  drawPathDelay: 100

  # How long a completed path is shown before the dots are cleared.
  clearDotsDelay: 500

  # Delay after the dots have been shrunk but before the dots start falling.
  dropDotsDelay: 150

  # Create a dots board in a root element. Calling this calls the dots of
  # the given grid to fall into place.
  constructor: (@root, @grid, next) ->
    @dots = ([] for r in [0..5])
    @dropDots @grid, next

  # Used for keeping track of the dot objects currently being used.
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
          nextRowToFill -= 1
      # Fill in the new dots.
      if nextRowToFill >= 0
        for r in [0..nextRowToFill]
          @newDot r, c
    setTimeout next, @fallingDotsDuration + @drawPathDelay

  # Create a new dot in the DOM. The dot is created in a "hidden row" above the
  # visibile grid, and is then moved to it's proper place via a nice CSS
  # animation that animates it falling into place.
  newDot: (r, c) ->
    dot = new Dot(r, c, @grid[r][c])
    dot.animateFall()
    @setDot r, c, dot
    @root.appendChild dot.element

  # Draw a path through the dots.
  drawPath: (@path, newGrid, next) ->
    # Recursively draw each path segment.
    drawNextSegment = (remainingPath, n, cycleCompleted = false) =>
      if @selectDot @getDot(remainingPath[0]...), not cycleCompleted
        cycleCompleted = true
        audio.playSquare n
      else if path.length is 1
        audio.playShrinker()
      else
        audio.playDot n
      # If there are at least two dots left, draw the next segment.
      if remainingPath.length > 1
        segment = @newPathSegment remainingPath[0], remainingPath[1]
        # Schedule the rest of the path to be drawn once the animation finishes.
        for event in ['animationend', 'webkitAnimationEnd']
          segment.addEventListener event, ->
            drawNextSegment remainingPath.slice(1), n + 1, cycleCompleted
        @root.appendChild segment
      # On the last dot, pause for a while and then shrink the dots.
      else if remainingPath.length is 1
        setTimeout @clearDots.bind(this, newGrid, next), @clearDotsDelay

    drawNextSegment path, 1

  # Quietly shrink some dots. This happens when there aren't any possible moves.
  shrinkRandom: (dots, newGrid, next) ->
    for [r, c] in dots
      @markDotForRemoval @getDot(r, c)
    @clearDots newGrid, next

  markDotForRemoval: (dot) ->
    if not dot.markedForDeletion
      dot.markedForDeletion = true
      @toBeRemoved ?= []
      @toBeRemoved.push dot

  # Mark a dot as selected. If the selected dot forms a cycle, it selects all
  # dots of that color like it does in the game. Returns whether a cycle was
  # completed.
  selectDot: (dot, checkForCycle = true) ->
    # Detect cycles by trying to reselect an already selected dot.
    if checkForCycle and dot.markedForDeletion
      return @completeSquare dot.color()
    @markDotForRemoval dot
    # Create a nice selection animation (removing an old one if necessary).
    dot.selection.element.remove() if dot.selection?
    dot.selection = new Dot(dot.row(), dot.col(), dot.color())
    dot.selection.animateSelection()
    @root.appendChild dot.selection.element
    return false

  # Complete a square, selecting all of the dots that ought to be popped.
  completeSquare: (color) ->
    @root.dataset.color = color
    # Shrink all dots of the same color.
    for r in [0..5]
      for c in [0..5]
        if (dot = @getDot r, c).color() is color
          @selectDot dot, false
    # Shrink the encircled dots.
    for dot in @getEncircledDots @path
      @selectDot dot, false

  # Find the dots that are encircled by a path. These dots are shrunk along
  # with the path when a square is made.
  getEncircledDots: (path) ->
    # Flood fill a grid using the path as the boundary. The dots not
    # reach are encircled by the path.
    visited = ([] for r in [0..5])
    visited[r][c] = true for [r, c] in path
    @floodFill visited
    # Create an array of the points that haven't been visited.
    encircled = []
    for r in [0..5]
      for c in [0..5]
        if not visited[r][c]
          encircled.push @getDot r, c
    encircled

  # Perform a flood fill starting from the edges of the grid.
  floodFill: (visited) ->
    flood = (r, c) ->
      # Don't continue if out of bounds or if already visited.
      if r < 0 or c < 0 or r > 5 or c > 5 or visited[r][c]
        return
      visited[r][c] = true
      # Flood each neighbor (including diagonals).
      for i in [-1..1]
        for j in [-1..1]
          if i isnt j
            flood r + i, c + j
    # Flood the border.
    for i in [0..5]
      flood i, 0
      flood i, 5
      flood 0, i
      flood 5, i
    visited

  # Create a path segment between two points. It will animate itself via CSS.
  newPathSegment: ([r1, c1], [r2, c2]) ->
    segment = document.createElement 'div'
    segment.className = "path-segment from-#{r1}-#{c1}-to-#{r2}-#{c2}"
    segment.dataset.color = @grid[r1][c1]
    # Make sure the segment will be removed at the end of the move.
    @toBeRemoved ?= []
    @toBeRemoved.push segment
    segment

  # Remove a path and the dots that were affected, then fill in the new dots.
  clearDots: (newGrid, next) ->
    delete @root.dataset.color if @root.dataset.color?
    # Remove all of the elements that we've indicated to be removed.
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

  row: (x) -> @element.dataset.row = x ? @element.dataset.row
  col: (x) -> @element.dataset.col = x ? @element.dataset.col
  color: (x) -> @element.dataset.color = x ? @element.dataset.color

  animateFall: ->
    @element.classList.add 'falling'
    @element.addEventListener 'animationend', (e) ->
      e.target.classList.remove 'falling'

  animateSelection: -> @animateAndRemove 'selecting'
  animateShrinking: -> @animateAndRemove 'shrinking'

  animateAndRemove: (className) ->
    @element.classList.add className
    @element.addEventListener 'animationend', (e) -> e.target.remove()


# Loads and plays sound assets.
audio = new class

  minDot: 1
  maxDot: 13
  minSquare: 4
  maxSquare: 12

  constructor: ->
    @shrinker = new Audio('assets/crunch.aif')
    @dots = []
    for i in [@minDot..@maxDot]
      @dots[i] = new Audio("assets/#{i}.aif")
    @squares = []
    for i in [@minSquare..@maxSquare]
      @squares[i] = new Audio("assets/square_#{i}.aif")

  playShrinker: -> @play @shrinker

  # Play the sound made by selecting the nth dot in a path. This starts by
  # going up in pitch back starts going back down again after the 13th dot.
  playDot: (n) ->
    numDots = @maxDot - @minDot + 1
    seqLength = 2 * numDots - 2    # Length of repeating sequence.
    seqIndex = (n - 1) % seqLength # Index within repeating sequence.
    i = if seqIndex < numDots then seqIndex else seqLength - seqIndex
    @play @dots[i + @minDot]

  # Play the sound made when a square is completed on its nth dot.
  playSquare: (n) ->
    @play @squares[Math.max @minSquare, Math.min(@maxSquare, n)]

  play: (audio) ->
    audio.currentTime = 0
    audio.play()
