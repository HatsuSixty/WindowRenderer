# WindowRenderer

Simple window server, written from scratch in C.

## Quick Start

To build the server and the test client:

```console
$ meson builddir
$ meson compile -C builddir
```

To test the server, run the following command **IN A TTY!**:

```console
$ ./build/src/WindowRenderer/WindowRenderer ./build/src/TestClient/TestClient
```

This will start the server, wait 5 seconds to start the client, and start rendering in the TTY. After 10 seconds, the client will close itself, and you can press `Ctrl + C` to stop the server.
