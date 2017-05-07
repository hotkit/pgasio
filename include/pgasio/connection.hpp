/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/network.hpp>


namespace pgasio {


    template<typename S>
    class connection;


    /// Perform a handshake that assumes to authentication or connection
    /// options are needed.
    template<typename S> inline
    connection<S> handshake(S &&socket, boost::asio::yield_context &yield) {
        return connection<S>(std::move(socket));
    }


    /// The connection to the database
    template<typename S>
    class connection {
        friend connection<S> handshake<>(S &&, boost::asio::yield_context &);

        S socket;

        connection(S &&s)
        : socket(std::move(s)) {
        }

    public:
    };


}

