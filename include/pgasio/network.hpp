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


    class decoder {
        byte_view buffer;
    public:
        decoder(byte_view b)
        : buffer(b) {
        }

        std::size_t remaining() const {
            return buffer.size();
        }

        unsigned char read_byte() {
            if ( not remaining() ) throw end_of_packet();
            const auto byte = buffer[0];
            buffer = buffer.slice(1);
            return byte;
        }

        int16_t read_int16() {
            return (read_byte() << 8) + read_byte();
        }
        int32_t read_int32() {
            return (read_byte() << 24) + (read_byte() << 16)
                + (read_byte() << 8) + read_byte();
        }

        byte_view read_bytes(std::size_t bytes) {
            if ( remaining() < bytes ) throw end_of_packet();
            auto ret = buffer.slice(0, bytes);
            buffer = buffer.slice(bytes);
            return ret;
        }

        byte_view read_string_view() {
            auto start = buffer.data();
            while ( read_byte() != 0 );
            return byte_view(start, buffer.data() - 1);
        }
        std::string read_string() {
            const auto view = read_string_view();
            return std::string(view.data(), view.data() + view.size());
        }
    };


    struct header {
        const char type;
        const std::size_t total_size;
        const std::size_t body_size;

        header(char t, std::size_t ts)
        : type(t), total_size(ts), body_size(ts - 4) {
        }

        template<typename S> inline
        decoder packet_body(S &socket, boost::asio::yield_context &yield) {
            assert(body.size() == 0 && body_size != 0);
            body = std::vector<unsigned char>(body_size);
            transfer(socket, body, body_size, yield);
            return byte_view(body);
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
            auto packet = head.packet_body(socket, yield);
            postgres_error::messages_type messages;
            while ( packet.remaining() > 1 ) {
                const auto type = packet.read_byte();
                messages[type] = packet.read_string();
            }
            throw postgres_error(std::move(messages));
        } else {
            return head;
        }
    }


}

