/**
    Copyright 2017-2018, Kirit Sælensminde. <https://kirit.com/pgasio/>
*/


#pragma once


#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>

#include <pgasio/memory.hpp>


namespace pgasio {


    /// A wrapper around another socket that adds a read buffer
    template<typename S>
    class buffered_socket {
        std::vector<unsigned char> buffer;
        raw_memory filled;

      public:
        /// The underlying socket that we're buffering
        S socket;

        /// Create a 2MB buffer for this data. Default to 96KB buffer
        explicit buffered_socket(S s, std::size_t size = 96 << 10)
        : buffer(size), filled{}, socket{std::move(s)} {}

        /// Make non-copyable
        buffered_socket(const buffered_socket &) = delete;
        buffered_socket &operator=(const buffered_socket &) = delete;
        /// Make movable
        buffered_socket(buffered_socket &&) = default;
        buffered_socket &operator=(buffered_socket &&) = default;

        /// Read & transfer the requested number of bytes to the buffer. The
        /// destination must be capable of storing the requested number of
        /// bytes.
        template<typename B, typename Y>
        void transfer(B &dest, std::size_t bytes, Y yield) {
            assert(bytes <= dest.size());
            raw_memory into{dest};
            while (bytes) {
                if (filled.size()) {
                    /// We have some bytes in the buffer we can use
                    const auto transfer = std::min(bytes, filled.size());
                    std::copy(
                            filled.data(), filled.data() + transfer,
                            into.data());
                    filled = filled.slice(transfer);
                    into = into.slice(transfer);
                    bytes -= transfer;
                } else {
                    // There is nothing in the buffer we can use
                    const auto got = socket.async_read_some(
                            boost::asio::buffer(buffer.data(), buffer.size()),
                            yield);
                    filled = raw_memory(buffer).slice(0, got);
                }
            }
        }

        /// Pass on methods to socket
        auto is_open() { return socket.is_open(); }

        template<typename... As>
        auto async_write_some(As &&... a) {
            return socket.async_write_some(std::forward<As>(a)...);
        }
    };


    /// Helper to creat a `buffered_socket` from a normal socket.
    template<typename S>
    inline auto make_buffered(S socket) {
        return buffered_socket<S>(std::move(socket));
    }


    /// Overload for the transfer function that uses the buffered socket.
    template<typename S, typename B, typename Y>
    inline void transfer(
            buffered_socket<S> &source, B &buffer, std::size_t bytes, Y yield) {
        source.transfer(buffer, bytes, yield);
    }


}
