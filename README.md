# pgasio

Postgres connectivity for modern C++

[![Build Status](https://travis-ci.org/KayEss/pgasio.svg?branch=master)](https://travis-ci.org/KayEss/pgasio)

Pgasio is designed for high performance connectivity to Postgres. It does only a few things, but it does them fast.

* Use of array views and string views to cut down on unnecessary copies and memory allocations.
* Only supports Unix domain sockets. If you want to connect remotely then pgPool is your friend.
* C++14, Boost ASIO and Boost co-routine to make the code fast efficient and easy to read.

Look at the [examples](./examples/) to see how it can be used, or read through the [header documentation](./include/pgasio/).


## Who should use pgasio

If you're making use of Postgres from C++ then you should still be using [libpqxx](http://pqxx.org/development/libpqxx/). However, if you really care about speed and are already using Boost (and new compilers) then using pgasio should be pretty easy, but it doesn't have a fraction of the features of libpqxx and it never will.

Currently only the simple query part of the Postgres protocol is handled. This is enough if all you need to do is to present text SQL to Postgres and have it return data to you.


### Benchmark results

Development has just started on pgasio, but the initial simple query based message exchnage already shows that it is possible to have a lot more performance. Initial benchmarking shows that pgasio is typically between twice as fast and ten times as fast as going through libpq and libpqxx.

After running `pgbench -i -s 10` the following times are observed, see the [examples](./examples/) for the `csj` and `stats` programs:

    $ time psql -c "SELECT * FROM pgbench_accounts" -A > /dev/null
    real 1.047
    user 0.640
    sys 0.112

    $ time -p csj "SELECT * FROM pgbench_accounts" > /dev/null
    SELECT to CSJ
    real 0.57
    user 0.53
    sys 0.14

With a larger dataset (`pgbench -i -s 100`) the following are achieved:

    $ time psql -c "SELECT * FROM pgbench_accounts" -A > /dev/null
    real 9.39
    user 5.70
    sys 1.08

    $ time csj "SELECT * FROM pgbench_accounts" > /dev/null
    SELECT to CSJ
    real 4.85
    user 4.96
    sys 1.23

The stats example is able to fetch multiple concurrent data sets in a single thread through the use of the coroutines:

    $ time bin/stats -c "SELECT * FROM pgbench_accounts" -c "SELECT * FROM pgbench_accounts" -c "SELECT * FROM pgbench_accounts"
    [....]
    Time: 5543ms
    Rows: 30000000 (blocks= 801)
    Rows per second: 5412231
    real 5.54
    user 3.85
    sys 1.15


### Testing pgasio

Travis is used to test Boost libraries from 1.60.0 to 1.64.0 together with clang 4.0 and gcc.


## Using pgasio

This is a header only library, so all you need to do is to clone the repository and then include the `include` folder on your include path :) If you're already using Boost then that's all you need.

The only (non-header) Boost libraries used are Coroutine and System -- these must be linked in to any executables you build.

The examples also make use of the `f5-threading` library (currently the experimental `feature/channel` branch). If you want to compile these you'll also need the include files from `f5-threading` on your include path, as well as the Boost libraries.

If you want to try it out and compile the examples you might find the [`get-boost`](./get-boost.md) and [`get-fost`](./get-fost.md) scripts useful to fetch and build a suitable set of dependancies.


## Roadmap

This is what we expect to happen given the likely release times of various compiler features, but if the coroutine TS and networking TS are available earlier then we'll change it accordingly.

* Pre-version 1 -- C++14 with Boost (current development). Supports only the simple query message exchange. The next development will focus on prepared statements.
* Version 1 -- C++14 with Boost.
* Version 2 -- C++17 with Boost.
* Version 3 -- Make use of the networking and co-routine TSs to remove the Boost dependency
* Version 4 -- C++20

