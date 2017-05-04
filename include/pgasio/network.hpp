/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/errors.hpp>
#include <pgasio/memory.hpp>

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
        const char type;
        const std::size_t total_size;
        const std::size_t body_size;

        header(char t, std::size_t ts)
        : type(t), total_size(ts), body_size(ts - 4) {
        }

        template<typename S> inline
        byte_view packet_body(S &socket, header head, boost::asio::yield_context &yield) {
            assert(body.size() == 0 && body_size != 0);
            body.reserve(body_size);
            transfer(socket, body, body_size, yield);
            return body;
        }
    private:
        std::vector<unsigned char> body;
    };


    template<typename S> inline
    auto packet_header(S &socket, boost::asio::yield_context &yield) {
        std::array<unsigned char, 5> buffer;
        transfer(socket, buffer, buffer.size(), yield);
        uint32_t bytes = (buffer[1] << 24) +
            (buffer[2] << 16) + (buffer[3] << 8) + buffer[4];
        header head{char(buffer[0]), bytes};
        if ( head.type == 'E' ) {
            throw postgres_error();
        } else {
            return head;
        }
    }


}

