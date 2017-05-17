/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/errors.hpp>
#include <pgasio/memory.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>

#include <array>


namespace pgasio {


    /// Transfer some number of bytes from the socket into the buffer
    template<typename S, typename B> inline
    void transfer(S &socket, B &buffer, std::size_t bytes, boost::asio::yield_context &yield) {
        boost::asio::async_read(socket, boost::asio::buffer(buffer.data(), buffer.size()),
            boost::asio::transfer_exactly(bytes), yield);
    };


    /// Decode information that comes in over the network according to the
    /// Postgres version 3 protocol.
    class decoder {
        byte_view buffer;
    public:
        decoder(byte_view b)
        : buffer(b) {
        }
        decoder(raw_memory b)
        : buffer(b.data(), b.size()) {
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


    /// An inbound packet header. The packet body must be read. For data row
    /// packets this would normally be done through the
    struct header {
        const char type;
        const std::size_t total_size;
        const std::size_t body_size;

        header()
        : type{}, total_size{}, body_size{} {
        }
        header(char t, std::size_t ts)
        : type(t), total_size(ts), body_size(ts - 4) {
        }

        template<typename S> inline
        std::vector<unsigned char> packet_body(S &socket, boost::asio::yield_context &yield) {
            std::vector<unsigned char> body(body_size);
            transfer(socket, body, body_size, yield);
            return body;
        }
    };


    /// Read a packet header, but not the packet body. If the packet is an
    /// error message then turn it into an exception.
    template<typename S> inline
    auto packet_header(S &socket, boost::asio::yield_context &yield) {
        std::array<unsigned char, 5> buffer;
        transfer(socket, buffer, buffer.size(), yield);
        uint32_t bytes = (buffer[1] << 24) +
            (buffer[2] << 16) + (buffer[3] << 8) + buffer[4];
        header head{char(buffer[0]), bytes};
        if ( head.type == 'E' ) {
            const auto body = head.packet_body(socket, yield);
            decoder packet{byte_view{body}};
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


    /// Helper to assemble a command packet to be sent to Postgres
    class command {
        char instruction;
        boost::asio::streambuf body;
    public:
        /// Construct a command. The very first packet sent after connecting
        /// doesn't have an instruction value. Send zero for this case. The
        /// `handshake` function will handle this for you.
        command(char type)
        : instruction(type) {
        }

        /// Add the specified bytes to the body
        template<typename B, typename = std::enable_if_t<std::is_integral<B>::value>>
        void bytes(array_view<B> av) {
            static_assert(sizeof(B) == 1, "Must add an array of bytes");
            body.sputn(av.data(), av.size());
        }

        /// Add a single byte to the body
        template<typename B, typename = std::enable_if_t<std::is_integral<B>::value>>
        void byte(B b) {
            static_assert(sizeof(B) == 1, "Must add a single byte");
            body.sputc(b);
        }

        /// Write an in8
        void int8(int8_t c) {
            byte(c);
        }
        /// Write an int16
        void int16(int16_t i) {
            int8((i & 0xff00) >> 8);
            int8(i & 0x00ff);
        }
        /// Write an int32
        void int32(int32_t i) {
            int8((i & 0xff00'0000) >> 24);
            int8((i & 0x00ff'0000) >> 16);
            int8((i & 0x0000'ff00) >> 8);
            int8(i & 0x0000'00ff);
        }

        /// Send a C string, including the terminating NIL character
        void c_str(const char *s) {
            while (*s) {
                byte(*s++);
            }
            int8(0);
        }


        /// Send the current command to Postgres. Return the body size sent
        template<typename S>
        std::size_t send(S &socket, boost::asio::yield_context &yield) {
            const std::size_t bytes = body.size() + 4;
            std::array<unsigned char, 5> h5;
            std::array<unsigned char, 4> h4;
            raw_memory header;
            std::size_t offset;
            if ( instruction ) {
                header = h5;
                header[0] = instruction;
                offset = 1;
            } else {
                header = h4;
                offset = 0;
            }
            header[offset + 0] = (bytes & 0xff00'0000) >> 24;
            header[offset + 1] = (bytes & 0x00ff'0000) >> 16;
            header[offset + 2] = (bytes & 0x0000'ff00) >> 8;
            header[offset + 3] = (bytes & 0x0000'00ff);
            std::array<boost::asio::streambuf::const_buffers_type, 2>
                data{{boost::asio::streambuf::const_buffers_type(header.data(), header.size()),
                    body.data()}};
            async_write(socket, data, yield);
            return body.size();
        }
    };


}

