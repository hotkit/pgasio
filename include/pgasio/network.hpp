/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>

#include <array>


namespace pgasio {


    template<typename S, typename B> inline
    void transfer(S &socket, B &buffer, std::size_t bytes, boost::asio::yield_context &yield) {
        boost::asio::async_read(socket, boost::asio::buffer(buffer.data(), buffer.size()),
            boost::asio::transfer_exactly(bytes), yield);
    };


    struct header {
        char type;
        std::size_t total_size;
        std::size_t body_size;
    };


    template<typename S> inline
    auto packet_header(S &socket, boost::asio::yield_context &yield) {
        std::array<unsigned char, 5> buffer;
        transfer(socket, buffer, buffer.size(), yield);
        uint32_t bytes = (buffer[1] << 24) +
            (buffer[2] << 16) + (buffer[3] << 8) + buffer[4];
        return header{char(buffer[0]), bytes, bytes - 4};
    }


}

