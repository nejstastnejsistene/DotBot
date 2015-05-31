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

### Run server
```
make
./dotbot
```

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
cd demo && coffee -wcm demo.js
```

Automatically compile the scss, including a source map

```
cd demo && scss --watch --sourcemap demo.scss:demo.css
```
