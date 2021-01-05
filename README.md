# cherry: A Minimal HTTP Server

![cherry image](cherry.png)

Inspired by the [Capriccio](http://capriccio.cs.berkeley.edu) project and the [Zaver](https://github.com/zyearn/zaver.git) HTTP server, cherry started out as an experimental project trying to incorporate cooperative threading (sometimes also known as fibers or coroutines) into a server. Unfortunately, as it turns out, it can be overly sophisticated to achieve the same level of performance as the Capriccio project did.

That said, cherry remains a simple HTTP server designed with unicore performance in mind and tries to boost efficiency by leveraging lightweight mechanisms such as FSM (finite state machine) scheduling and I/O multiplexing (epoll). More importantly, it, hopefully, provides a working example for many of the ideas described in books but maybe not talked about as much anymore. Anyway, perhaps cherry will never be a serious server whatsoever, but from what I can see now, with its simplicity, it can still be a perfect playground for testing new ideas.

## Basic design

The basic idea behind cherry is that for unicore servers, there is only one single control flow at all times, and if we can eliminate blocking operations, even without preemptive threading, it's still possible for us to implement almost the same level of concurrency as cooperative threading.
Currently, cherry uses

- `epoll(7)` to leverage I/O multiplexing  

- a queue to store file descriptors that are ready for I/O operations.

- a finite state machine to schedule (e.g. when jobs are waiting in the queue, `epoll(7)` should not block)

With that being said, since we use the edge-triggered mode of `epoll(7)`, we must read through the end of a file descriptor in one go every time, which may also lead to starvation of other waiting file descriptors.

## Build

To build cherry, you simply run `make` from `src`.

## System requirements

Since cherry uses `epoll(7)`, a Linux-specific system call, it runs only on Linux systems.

## TODO

- Configuration file

- Build-time single-/multi-thread option

- Installation

- Support for `kqueue`

- Better data structure for `task_set`

## LICENSE

Like many projects of a similar nature, cherry uses a large amount of code from books and websites, and a significant part of the source code used in parsing HTTP requests has come directly from Zaver. For what it's worth, despite its lack of any actual originality, cherry itself as a whole is shamelessly released under the MIT license (while packages used in this project may use a different license).

```plaintext
Copyright 2021 Zee

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

```
