# pgasio

Postgres connectivity for modern C++

Pgasio is designed for high performance connectivity to Postgres. It does only a few things, but it does them fast.

* Use of array views and string views to cut down on unnecessary copies and memory allocations.
* Only supports Unix domain sockets. If you want to connect remotely then pgPool is your friend.
* C++14, Boost ASIO and Boost co-routine to make the code fast efficient and easy to read.

## Roadmap

* Version 1 -- C++14. The current development is heading here.
* Version 2 -- C++17.
* Version 3 -- Make use of the networking and co-routine TSs to remove the Boost dependency
* Version 4 -- C++20

