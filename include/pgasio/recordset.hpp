/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/memory.hpp>
#include <pgasio/network.hpp>

#include <boost/asio/spawn.hpp>


namespace pgasio {


    /// Store a block of data rows. The data buffer is non-aligned.
    class record_block {
        std::size_t columns;
        std::vector<byte_view> records;
        unaligned_slab buffer;

    public:
        /// The block is intiialised to hold a number of bytes of data row
        /// network packets and indexes the columns into those through
        /// the records member;
        record_block(std::size_t column_count,
            std::size_t bytes = 4u << 20, // 4MB of record data
            std::size_t record_size = 2 << 11) // 2KB mean record size
        : columns(column_count), buffer(bytes) {
            const auto expected_records = (bytes + record_size - 1) / record_size;
            records.reserve(columns * expected_records);
        }

        /// Not copyable
        record_block(const record_block &) = delete;
        record_block &operator = (const record_block &) = delete;

        /// The number of bytes for row data still available in this block
        std::size_t remaining() const {
            return buffer.remaining();
        }

        /// Read the next data packet into the block
        template<typename S>
        void read_data_row(S &socket, std::size_t bytes, boost::asio::yield_context &yield) {
            assert(bytes <= remaining());
            auto packet_data = buffer.allocate(bytes);
            transfer(socket, packet_data, bytes, yield);
            // TODO: Decode the columns
        }
    };


}

