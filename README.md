# http
For now, template HTTP 1.1 implementation. Always returns 200. Assembled this on a whim, might use this for something cool later.
# Get started
Console 1 for compiling and running the server.
```console
$ cd http/src
$ make
$ bin/main
Connected to server at 127.0.0.1:6969
Server listening on port 6969...

```
Console 2 for feeding requests.
```console
$ curl localhost:6969
... 
```
Or open a browser and open `localhost:6969`.
# Environment
This program uses POSIX threads (pthreads), supported on POSIX-compliant systems such as Linux and macOS. If working on Windows, it may require additional libraries (TBA).
