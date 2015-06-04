DotBot
======

### Install dependencies and build
```
sudo apt-get install libwebsockets-dev libjson0-dev
make
```
If you're not running Ubuntu, check out [heroku-buildpack-DotBot](https://github.com/nejstastnejsistene/heroku-buildpack-DotBot) for an idea of how to do this the hard way.

### Run benchmark
```
$ ./benchmark
1000 games of 35 turns (with shrinkers)
=======================================

total time:   580.633484s
average time: 0.580633s

mean:    345.368011
std dev: 36.543896
minimum: 225
median:  346
maximum: 458
```

### Run server
```
./server
```

### Run web client
```
make -C demo
python -m SimpleHTTPServer 8000
# visit http://localhost:8000/demo/ in your browser
```

### Commands to run while developing the demo

Run the websocket server

```
./server
```
Serve the client code

```
python -m SimpleHTTPServer 8000
```

Automatically compile the coffeescript, including a source map

```
cd demo && coffee -wcm main.coffee
```

Automatically compile the scss, including a source map

```
cd demo && scss --watch --sourcemap main.scss:main.css
```
