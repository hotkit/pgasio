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
    connection<S> handshake(
        S socket,
        const char *user, const char *database,
        boost::asio::yield_context &yield
    ) {
        command cmd{0}; // The initial connect doesn't use a packet type char
        cmd.int32(0x0003'0000);
        cmd.c_str("user");
        cmd.c_str(user);
        if ( database != nullptr && database[0] != 0 ) {
            cmd.c_str("database");
            cmd.c_str(database);
        }
        cmd.int8(0);
        cmd.send(socket, yield);
        std::unordered_map<std::string, std::string> settings;
        return connection<S>(std::move(socket), std::move(settings));
    }


    /// The connection to the database
    template<typename S>
    class connection {
        friend connection<S> handshake<>(S, const char *, const char *, boost::asio::yield_context &);

        std::unordered_map<std::string, std::string> settings;

        connection(S s, std::unordered_map<std::string, std::string> set)
        : settings(std::move(set)), socket(std::move(s)) {
        }

    public:
        S socket;
    };


}

