# WindowRenderer

Simple window server, written from scratch in C.

## Quick Start

To build the server and the test client:

```console
$ meson builddir
$ meson compile -C builddir
```

To test the server, create a shell script with the code:

```bash
#!/bin/sh

./builddir/src/WindowRenderer/WindowRenderer &
sleep 1
./builddir/src/TestClient/TestClient &
sleep 15
killall --signal SIGINT WindowRenderer
```

And run the script **in a TTY!**. This will start the server, open a client after 1 second, and terminate the server after 15 seconds. The client closes itself automatically after 10 seconds.