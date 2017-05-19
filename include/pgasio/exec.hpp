/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/connection.hpp>
#include <pgasio/recordset.hpp>


namespace pgasio {


    /// A description of a recrodset column
    struct column_meta {
        std::string name;
        int32_t table_oid;
        int16_t table_column;
        int32_t field_type_oid;
        int16_t data_size;
        int32_t type_modifier;
        int16_t format_code;
    };


    /// A wrapper around the basic block delivery mechanism
    template<typename S>
    class recordset {
        connection<S> &cnx;
        std::size_t next_row_data_size;
    public:
        recordset(connection<S> &cnx, header c, boost::asio::yield_context &yield)
        : cnx(cnx), next_row_data_size{}, columns([&]() {
                std::vector<column_meta> columns;
                auto packet = c.packet_body(cnx.socket, yield);
                decoder description(packet);
                const auto count = description.read_int16();
                columns.reserve(count);
                while ( columns.size() != count ) {
                    column_meta col;
                    col.name = description.read_string();
                    col.table_oid = description.read_int32();
                    col.table_column = description.read_int16();
                    col.field_type_oid = description.read_int32();
                    col.data_size = description.read_int16();
                    col.type_modifier = description.read_int32();
                    col.format_code = description.read_int16();
                    columns.push_back(col);
                }
                return columns;
            }())
        {
            while ( cnx.socket.is_open() ) {
                auto header = pgasio::packet_header(cnx.socket, yield);
                switch ( header.type ) {
                case 'D':
                    next_row_data_size = header.body_size;
                    return;
                default:
                    throw std::runtime_error(
                        std::string("Wasn't expecting this packet type: ") + header.type);
                }
            }
            throw std::runtime_error("Connection closed before getting a recordset back");
        }

        const std::vector<column_meta> columns;

        pgasio::record_block next_block(boost::asio::yield_context &yield) {
            if ( next_row_data_size ) {
                pgasio::record_block block{columns.size()};
                next_row_data_size = block.read_rows(cnx.socket, next_row_data_size, yield);
                return block;
            } else {
                return pgasio::record_block{};
            }
        }
    };


    /// A wrapper that allows access to the recordsets
    template<typename S>
    class resultset {
        connection<S> &cnx;
        header current;
    public:
        resultset(connection<S> &cnx, header c)
        : cnx(cnx), current(c) {
        }

        pgasio::recordset<S> recordset(boost::asio::yield_context &yield) {
            assert(current.type == 'T');
            return pgasio::recordset<S>{cnx, current, yield};
        }
    };


    /// Execute an SQL command and return the results
    template<typename S>
    inline resultset<S> exec(connection<S> &cnx, const char *sql, boost::asio::yield_context &yield) {
        command query('Q');
        query.c_str(sql);
        query.send(cnx.socket, yield);
        while ( cnx.socket.is_open() ) {
            auto header = pgasio::packet_header(cnx.socket, yield);
            switch ( header.type ) {
            case 'T':
                return resultset<S>{cnx, header};
            default:
                throw std::runtime_error(std::string("Wasn't expecting this packet type: ") + header.type);
            }
        }
        throw std::runtime_error("Connection closed before getting a recordset back");
    }


}

