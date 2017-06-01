# Examples

To build these you will need the `f5-threading` library and a few Boost libraries. The [`.travis.yml`](./../.travis.yml) file contains hints on how to get them and to do a build using [Boost build](http://www.boost.org/build/).

For all of these examples the following options set Postgres connection details:

* `-h` -- Full path to the Unix domain socket. Normally Postgres takes only the directory, but csj requires the full filename. The default is `/var/run/postgresql/.s.PGSQL.5432`.
* `-U` -- User name. Defaults to the `LOGNAME` environment variable.
* `-d` -- Database name. Defaults to nothing, and the Postgres server defaults a missing database option to the user name.


## [stats](./stats.cpp#L6)

This minimal example produces some statistics (hence `stats`) about results from giving SQL to the database server.  It also shows how to iterate over one or more statements that can produce multiple recordsets.

All of the coroutines used to service the connections run in the same thread (the main thread), so this example can be used to see how many connections a single thread can service for different statements given to the database server.

The option `-c` specifies a command that is to be run (and it may be provided multiple times). Otherwise pass in the filenames of SQL commands to be run.

Example:

    stats -c "SELECT 1, 2" pgbench.sql

Run the `SELECT` statement in one coroutine and all of the SQL found in the `pgbench.sql ` file in another.

    stats pgbench.sql pgbench.sql pgbench.sql

Run three coroutines performing the SQL in the `pgbench.sql` file.


## [csj](./f5/csj.cpp#L6)

This shows how to use the basic parts of the libraries to efficiently fetch data from Postgres, do some light processing and then output the data. It demonstrates how the _f5-threading_ library can be used to handle record data over multiple threads.

The CSJ format is described at _[Comma Separated JSON](http://www.kirit.com/Comma%20Separated%20JSON)_.

Example:

    csj -h /run/postgresql/.s.PGSQL.5433 "SELECT * FROM pgbench_accounts"

Connects to the database server whose unix domain socket is at the file `/run/postgresql/.s.PGSQL.5433` and displays all of the data in the `pgbench_accounts` table.
