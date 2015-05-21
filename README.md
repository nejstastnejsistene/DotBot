DotBot
======

Compile
```
git clone git://git.libwebsockets.org/libwebsockets
cd websockets
cmake .
make
cd ..
make
```

Run server
```
LD_LIBRARY_PATH=libwebsockets/lib ./dotbot
```

Run web client
```
python -m SimpleHTTPServer 8000
# visit http://localhost:7681/test.html in your browser
```
