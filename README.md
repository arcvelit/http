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

Client connected from 127.0.0.1:33630
Received: GET / HTTP/1.1
Host: localhost:6969
User-Agent: curl/8.11.1
Accept: */*


```
Console 2 for feeding requests.
```console
$ curl localhost:6969
We'll be in touch!
$ 
```
# Environment
This program uses POSIX threads (pthreads), supported on POSIX-compliant systems such as Linux and macOS. If working on Windows, it may require additional libraries (TBA).
