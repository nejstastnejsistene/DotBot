DotBot
======

### Install libwebsockets
```
git clone git://git.libwebsockets.org/libwebsockets
cd libwebsockets
cmake .
make
sudo make install
```

### Install libjson-c
```
git clone git://github.com/json-c/json-c
cd json-c
sh autogen.sh
./configure
make
sudo make install
```

### Build
```
make
make debug # With debugging and profiling info
```

Check out [heroku-buildpack-DotBot](https://github.com/nejstastnejsistene/heroku-buildpack-DotBot) to see how this is done on Heroku.

### Run benchmark
```
$ ./benchmark
1000 games of 35 turns (without shrinkers)
==========================================

total time:            943.786621s
average time per game: 0.943787s
average time per move: 26.965332ms

mean:    310.463013
std dev: 47.516216
minimum: 99
median:  312
maximum: 427
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
