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
