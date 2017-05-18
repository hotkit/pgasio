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
    class recordset {
        header current;
    public:
        recordset(header c, boost::asio::yield_context &yield)
        : current(c) {
        }

        const std::vector<column_meta> columns;
    };


    /// A wrapper that allows access to the recordsets
    class resultset {
        header current;
    public:
        resultset(header c)
        : current(c) {
        }

        pgasio::recordset recordset(boost::asio::yield_context &yield) {
            return pgasio::recordset{current, yield};
        }
    };


    /// Execute an SQL command and return the results
    template<typename S>
    inline resultset exec(connection<S> &cnx, const char *sql, boost::asio::yield_context &yield) {
        using namespace std::string_literals;

        command query('Q');
        query.c_str(sql);
        query.send(cnx.socket, yield);
        while ( cnx.socket.is_open() ) {
            auto header = pgasio::packet_header(cnx.socket, yield);
            switch ( header.type ) {
            case 'T':
                return resultset{header};
            default:
                throw std::runtime_error("Wasn't expecting this packet type: "s + header.type);
            }
        }
        throw std::runtime_error("Connection closed before getting a recordset back");
    }


}

