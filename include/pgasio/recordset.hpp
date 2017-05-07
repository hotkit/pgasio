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
        explicit record_block(std::size_t column_count,
            std::size_t bytes = 4u << 20, // 4MB of record data
            std::size_t record_size = 2 << 11) // 2KB mean record size
        : columns(column_count), buffer(bytes) {
            const auto expected_records = (bytes + record_size - 1) / record_size;
            records.reserve(columns * expected_records);
        }

        /// Not copyable
        record_block(const record_block &) = delete;
        record_block &operator = (const record_block &) = delete;
        /// Moveable
        record_block(record_block &&) = default;
        record_block &operator = (record_block &&) = default;

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
            decoder packet(packet_data);
            [[maybe_unused]] const auto cols = packet.read_int16();
            assert(cols == columns);
            while ( packet.remaining() ) {
                const auto bytes = packet.read_int32();
                if ( bytes == -1 ) {
                    records.push_back(byte_view());
                } else {
                    records.push_back(packet.read_bytes(bytes));
                }
            }
            assert(records.size() % cols == 0);
        }

        /// Fill the block with data. Return zero if there is no more data to come
        template<typename S>
        std::size_t read_rows(S &socket, std::size_t bytes, boost::asio::yield_context &yield) {
            do {
                read_data_row(socket, bytes, yield);
                auto next = packet_header(socket, yield);
                if ( next.type == 'D' ) {
                    bytes = next.body_size;
                } else if ( next.type == 'C' ) {
                    next.packet_body(socket, yield);
                    auto finish{packet_header(socket, yield)};
                    if ( finish.type == 'Z' ) {
                        finish.packet_body(socket, yield);
                        return 0;
                    } else {
                        throw std::logic_error(std::string("Expected Z after C, but got: ") + finish.type);
                    }
                } else {
                    throw std::logic_error(std::string("Unknown packet type: ") + next.type);
                }
            } while ( bytes <= remaining() );
            return bytes;
        }

        /// Return the current record fields
        array_view<const byte_view> fields() const {
            return records;
        }
    };


}

