# pgasio

Postgres connectivity for modern C++

Pgasio is deisgned for high performance connectivity to Postgres. It does only a few things, but it does them fast.

* Use of array views and string views to cut down on unnecessary copies and memory allocations.
* Only supports Unix domain sockets. If you want to connect remotely then pgPool is your friend.
* C++14, Boost ASIO and Boost coroutine to make the code fast efficient and easy to read.

## Roadmap

* Version 1 -- The current development, with C++14.
* Verison 2 -- Swtich to C++17.
* Version 3 -- Make use of the networking and coroutine TSs to remove the Boost dependancy
* Version 4 -- C++20

