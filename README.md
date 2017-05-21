# pgasio

Postgres connectivity for modern C++

[![Build Status](https://travis-ci.org/KayEss/pgasio.svg?branch=master)](https://travis-ci.org/KayEss/pgasio)

Pgasio is designed for high performance connectivity to Postgres. It does only a few things, but it does them fast.

* Use of array views and string views to cut down on unnecessary copies and memory allocations.
* Only supports Unix domain sockets. If you want to connect remotely then pgPool is your friend.
* C++14, Boost ASIO and Boost co-routine to make the code fast efficient and easy to read.

Look at the [examples](./examples/) to see how it can be used, or read through the [header documentation](./include/pgasio/).


## Roadmap

* Version 1 -- C++14. (Current development)
* Version 2 -- C++17.
* Version 3 -- Make use of the networking and co-routine TSs to remove the Boost dependency
* Version 4 -- C++20

