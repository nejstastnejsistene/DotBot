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
python -m SimpleHTTPServer 8000
# visit http://localhost:8000/test.html in your browser
```
