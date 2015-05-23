DotBot
======

Install libwebsockets
```
git clone git://git.libwebsockets.org/libwebsockets
cd libwebsockets
cmake .
make
sudo make install
```

Run server
```
make
./dotbot
```

Run web client
```
make -C demo
python -m SimpleHTTPServer 8000
# visit http://localhost:8000/demo/demo.html in your browser
```
