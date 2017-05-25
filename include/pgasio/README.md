# pgasio Headers

To get started look at [`connection.hpp`](./connection.hpp#L9) to connect to the database and then at [`exec.hpp`](./exec.hpp#L9) to see how to send a `SELECT` to Postgres and traverse the resulting record set. [`csj.cpp`](./../../examples/csj.cpp) contains a full example.

For simple interactions with Postgres the general process is:

1. Create a connection --  [`connection.hpp`](./connection.hpp#L9)
2. Issue a command to Postgres -- [`exec.hpp`](./exec.hpp#L9), or [`network.hpp`](./network.hpp#L9) then possibly [`record_block.hpp`](./record_block.hpp#L9).
3. Iterate through the messages Postgres returns until a `Z` packet is returned -- you can then return to step 2 if you need to.

It's important that the resultant data is properly iterated though because the network traffic must be fully read.


## [`buffered.hpp`](./buffered.hpp#L9)

A wrapper for a socket type which adds a read buffer. The read buffering is especially useful for fetching data where there are many small rows as it reduces the number of system calls needed.

Use `make_buffered` to return a `buffered_socket` instance that can be used in the rest of the APIs as a drop in replacement for a normal Boost ASIO socket.


## [`connection.hpp`](./connection.hpp#L9)

A connection to a Postgres server can be created by calling `handshake` having already opened a suitable socket for it to use. This will return a `connection` object which will contain information provided by the server duing the connection set up.

There is also a helper function, `unix_domain_socket`, which can be used to create a suitable unix domain socket to pass to `handshake`:

    #include <pgasio/buffered.hpp>
    #include <pgasio/connection.hpp>

    auto cnx = pgasio::handshake(
        pgasio::make_buffered(
            pgasio::unix_domain_socket(ioservice, "/path/to/socket", yield)),
        "myusername", "somedb", yield);


## [`errors.hpp`](./errors.hpp#L9)

Contains classes that can be used as either error returns or exceptions.

* `postgres_error` -- This is thrown by `packet_header` when Postgres returns an error to the client.
* `end_of_packet` -- Thrown by the `decoder` when there aren't enough bytes left in the message.


## [`exec.hpp`](./exec.hpp#L9)

The `exec` function can be used to send a query to Postgres. This returns a `resultset` instance from which `recordset`s can be fetched. The `recordset` will deliver `record_block` instances (see [`record_block.hpp`](./record_block.hpp#L9)) from which rows can be decoded using the information in the `columne_meta` structures held by the `recordset`.


## [`memory.hpp`](./memory.hpp#L9)

Contains an `array_view` implementation that can be used to share and manipulate contiguous blocks of memory without owning them. There are specialisations for byte arrays: `byte_view` is immutable and `raw_memory` is mutable. Wherever possible other interfaces are specified in terms of `array_view`s, see `decoder` for an example.

`unaligned_slab` is an owner for a block of memory that unaligned sections can be fetched from. These sub-parts can be safely used as


## [`network.hpp`](./network.hpp#L9)

Information about part of a reply from Postgres, a message/packet, is returned by the `packet_header` function. The returned `header` structure can be used to fetch a new `std::vector` of byte values for the body which can then be decoded by a `decoder` instance. The [message formats are described in the Postgres documentation](https://www.postgresql.org/docs/current/static/protocol-message-formats.html).

A `decoder` takes a view of some memory so can be used with most memory allocation strategies:

* Single bytes are read using `read_byte`.
* 16 and 32 bit signed integers are read using `read_int16` and `read_int32` respectively.
* A fixed length sequence of bytes can be read using `read_bytes`.
* Both `read_string_view` and `read_string` will read a NIL terminated string differing only in their return type (the first returns a `byte_view` and the latter allocates and returns a `std::string`).

`command` instances can be used to construct messages and then send them to Postgres.

The `transfer` helper function allows a certain number of bytes to be fetched from the socket and placed in memory views, e.g. `raw_memory` view or a `std::vector<char>`.

For an example of how these can be used together see the `handshake` implementation in [`connection.hpp`](./connection.hpp#L21).

## [`record_block.hpp`](./record_block.hpp#L9)

Contains a block of record data fetched from a socket. It owns a large slab of memory (an `unaligned_slab`) together with a vector of field data locations so the column data can be interpreted correctly.

