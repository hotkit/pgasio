/**
    Copyright 2017-2018, Kirit Sælensminde. <https://kirit.com/pgasio/>
*/


#pragma once


#include <pgasio/connection.hpp>
#include <pgasio/record_block.hpp>


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
        std::vector<column_meta> cols;
        std::size_t next_row_data_size;
        bool sentinel;

      public:
        /// Return a sentinel recordset that shows the resultset is done
        template<typename Y>
        recordset(connection<S> &cnx, Y)
        : cnx(cnx), next_row_data_size{}, sentinel{true} {}

        /// Return a rea; recordset that may contains data
        template<typename Y>
        recordset(connection<S> &cnx, header c, Y yield)
        : cnx(cnx),
          cols([&]() {
              std::vector<column_meta> columns;
              auto body = c.message_body(cnx.socket, yield);
              decoder description(body);
              if (c.type == 'T') {
                  const std::size_t count = description.read_int16();
                  columns.reserve(count);
                  while (columns.size() != count) {
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
              }
              return columns;
          }()),
          next_row_data_size{},
          sentinel{false} {
            /// We can get recordsets that are completely empty. In this
            // /case we'll get an `I` message type, _EmptyQueryResponse_
            /// and there won't be field infromation or anything else. This
            /// happens, for example, when an empty SQL statement is presented
            /// to the database.
            while (c.type == 'T' && cnx.socket.is_open()) {
                auto header = message_header(cnx.socket, yield);
                switch (header.type) {
                case 'C':
                    header.message_body(cnx.socket, yield);
                    next_row_data_size = 0u;
                    return;
                case 'D': next_row_data_size = header.body_size; return;
                default:
                    throw std::runtime_error(
                            "Wasn't expecting this message type: "
                            + std::to_string(int(header.type)) + '/'
                            + header.type);
                }
            }
            if (c.type == 'I') {
                next_row_data_size = 0u;
            } else {
                throw std::runtime_error(
                        "Connection closed before getting a recordset back");
            }
        }

        /// The column meta data for this recordset.
        array_view<const column_meta> columns() const { return cols; }

        /// Returns true if the recordset is one that contains data, i.e. not
        /// the sentinel value representing the end of the data.
        operator bool() const { return not sentinel; }

        /// Returns the next data block
        template<typename Y>
        pgasio::record_block next_block(Y yield) {
            if (next_row_data_size) {
                pgasio::record_block block{cols.size()};
                next_row_data_size =
                        block.read_rows(cnx.socket, next_row_data_size, yield);
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

      public:
        resultset(connection<S> &cnx) : cnx(cnx) {}

        template<typename Y>
        pgasio::recordset<S> recordset(Y yield) {
            while (cnx.socket.is_open()) {
                auto header = message_header(cnx.socket, yield);
                switch (header.type) {
                case 'I':
                case 'T': return pgasio::recordset<S>{cnx, header, yield};
                case 'Z':
                    header.message_body(cnx.socket, yield);
                    return pgasio::recordset<S>(cnx, yield);
                default:
                    throw std::runtime_error(
                            "Fetching next recordset wasn't expecting this "
                            "message type: "
                            + std::to_string(header.type) + "/" + header.type);
                }
            }
            throw std::runtime_error(
                    "Connection closed before getting a recordset back");
        }
    };


    /// Execute an SQL command and return the results
    template<typename S, typename Y>
    inline resultset<S> query(connection<S> &cnx, const char *sql, Y yield) {
        command query('Q');
        query.c_str(sql);
        query.send(cnx.socket, yield);
        return resultset<S>{cnx};
    }
    template<typename S, typename Y>
    inline resultset<S>
            query(connection<S> &cnx, const std::string &sql, Y yield) {
        return query(cnx, sql.c_str(), yield);
    }


}
