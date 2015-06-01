DotBot
======

### Install dependencies, build, and run server
```
sudo apt-get install libwebsockets-dev libjson0-dev
make
./dotbot
```
If you're not running Ubuntu, check out [heroku-buildpack-DotBot](https://github.com/nejstastnejsistene/heroku-buildpack-DotBot) for an idea of how to do this the hard way.

### Run web client
```
make -C demo
python -m SimpleHTTPServer 8000
# visit http://localhost:8000/demo/demo.html in your browser
```

### Commands to run while developing the demo

Run the websocket server

```
./dotbot
```
Serve the client code

```
python -m SimpleHTTPServer 8000
```

Automatically compile the coffeescript, including a source map

```
cd demo && coffee -wcm demo.coffee
```

Automatically compile the scss, including a source map

```
cd demo && scss --watch --sourcemap demo.scss:demo.css
```
