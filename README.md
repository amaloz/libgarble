# libgarble
Garbling library based on [JustGarble](http://cseweb.ucsd.edu/groups/justgarble/).

This code is still in alpha and the API is not yet fully stable (as in, future commits may change it).  However, I'd love to hear feedback from anyone who uses it, to help improve the library.

## Installation instructions

Run the following:
```
./configure
make
sudo make install
```
This installs two libraries: `libgarble` for actually garbling and evaluating a circuit, and `libgarblec` for writing circuits.

To test, run the following:
```
cd test
make aes
./aes
```

This will garble and evaluate an AES circuit, and present timings in cycles/gate.
