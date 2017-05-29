# Examples

To build these you will need the `f5-threading` library and a few Boost libraries. The [`.travis.yml`](./../.travis.yml) file contains hints on how to get them and to do a build using [Boost build](http://www.boost.org/build/).

For all of these examples the following options set Postgres connection details:

* `-h` -- Full path to the Unix domain socket. Normally Postgres takes only the directory, but csj requires the full filename. The default is `/var/run/postgresql/.s.PGSQL.5432`.
* `-U` -- User name. Defaults to the `LOGNAME` environment variable.
* `-d` -- Database name. Defaults to nothing, and the Postgres server defaults a missing database option to the user name.


## [stats](./stats.cpp#L6)

This minimal example shows how to iterate over one or more statements that can produce multiple recordsets. Statistics about the executed SQL is returned.

The option `-c` specifies a command that is to be run (and it may be provided multiple times). Otherwise pass in the filenames of SQL commands to be run.

Example:

    stats -c "SELECT 1, 2" pgbench.sql

## [csj](./csj.cpp#L6)

This shows how to use the basic parts of the libraries to efficiently fetch data from Postgres, do some light processing and then output the data. It demonstrates how the _f5-threading_ library can be used to handle record data over multiple threads.

The CSJ format is described at _[Comma Separated JSON](http://www.kirit.com/Comma%20Separated%20JSON)_.

Example:

    csj -h /run/postgresql/.s.PGSQL.5433 "SELECT * FROM pgbench_accounts"

