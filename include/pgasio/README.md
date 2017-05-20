# pgasio Headers


## [`connection.hpp`](./connection.hpp#L9)

A connection to a Postgres server can be created by calling `handshake` having already opened a suitable socket for it to use. This will return a `connection` object which will contain information provided by the server duing the connection set up.

There is also a helper function, `unix_domain_socket`, which can be used to create a suitable unix domain socket to pass to `handshake`:

    auto cnx = pgasio::handshake(
        pgasio::unix_domain_socket(ioservice, "/path/to/socket", yield),
        "myusername", "somedb", yield);


## [`errors.hpp`](./errors.hpp#L9)

## [`exec.hpp`](./exec.hpp#L9)

## [`memory.hpp`](./memory.hpp#L9)

## [`network.hpp`](./network.hpp#L9)

Information about part of a reply from Postgres, a message/packet, is returned by the `packet_header` function. The returned `header` structure can be used to fetch a new `std::vector` of byte values for the body which can then be decoded by a `decoder` instance. The [message formats are described in the Postgres documentation](https://www.postgresql.org/docs/current/static/protocol-message-formats.html).

A `decoder` takes a view of some memory so can be used with most memory allocation strategies:

* Single bytes are read using `read_byte`.
* 16 and 32 bit signed integers are read using `read_int16` and `read_int32` respectively.
* A fixed length sequence of bytes can be read using `read_bytes`.
* Both `read_string_view` and `read_string` will read a NIL terminated string differing only in their return type (the first returns a `byte_view` and the latter allocates and returns a `std::string`).

`command` instances can be used to construct messages and then send them to Postgres.

The `transfer` helper function allows a certain number of bytes to be fetched from the socket and placed in memory views, e.g. `raw_memory` view or a `std::vector<char>`.


## [`recordset.hpp`](./recordset.hpp#L9)

