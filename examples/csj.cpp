/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#include <cstdlib>
#include <experimental/iterator>
#include <iostream>
#include <string>

#include <f5/threading/channel.hpp>
#include <f5/threading/reactor.hpp>
#include <f5/threading/sync.hpp>

#include <pgasio/exec.hpp>


/// Append safe data (numeric so nothing to do)
std::string &safe_data(std::string &str, pgasio::byte_view text) {
    return str.append(reinterpret_cast<const char *>(text.data()), text.size());
}
/// Append a safe string (no weird JSON escaping needed)
std::string &safe_string(std::string &str, pgasio::byte_view text) {
    return safe_data(str += '"', text) += '"';
}
/// Append an unsafe string (with escaping)
std::string &json_string(std::string &str, pgasio::byte_view text) {
    str += '"';
    while ( text.size() ) {
        switch ( text[0] ) {
        case '\n':
            str += "\\n";
            break;
        case '\r':
            str += "\\r";
            break;
        case '\t':
            str += "\\t";
            break;
        case '\\':
            str += "\\\\";
            break;
        case '\"':
            str += "\\\"";
            break;
        default:
            str += text[0];
        }
        text = text.slice(1);
    }
    return str += '"';
}


int main(int argc, char *argv[]) {
    std::cerr << "SELECT to CSJ" << std::endl;

    /// The parameters we need to use. This is all legacy C stuff :(
    const char *user = std::getenv("LOGNAME");
    const char *database = nullptr;
    const char *path = "/var/run/postgresql/.s.PGSQL.5432";
    const char *sql = nullptr;

    /// Go through the command line and pull the details out
    for ( std::size_t a{1}; a < argc; ++a ) {
        using namespace std::string_literals;
        auto read_opt = [&](char opt) {
            if ( ++a >= argc ) throw std::runtime_error("Missing option after -"s + opt);
            return argv[a];
        };
        if ( argv[a] == "-d"s ) {
            database = read_opt('d');
        } else if ( argv[a] == "-h"s ) {
            path = read_opt('h');
        } else if ( argv[a] == "-U"s ) {
            user = read_opt('U');
        } else if ( argv[a][0] == '-' ) {
            std::cerr << "Unknown command line option: "s + argv[a][0] << std::endl;
            return 2;
        } else if ( sql ) {
            std::cerr << "Extra SQL command ignored\n" << argv[a] << std::endl;
        } else {
            sql = argv[a];
        }
    }
    if ( not sql ) {
        std::cerr << "MIssing SQL statement" << std::endl;
        return 1;
    }

    /// Set up the reactor thread pool and the channels we'll need
    f5::boost_asio::reactor_pool reactor{[]() {
        std::cerr << "An error occured\n";
        auto ep = std::current_exception();
        if ( ep ) {
            try {
                std::rethrow_exception(ep);
            } catch ( std::exception &e ) {
                std::cerr << e.what() << std::endl;
            } catch ( ... ) {
                std::cerr << "Unknown exception" << std::endl;
            }
        } else {
            std::cerr << "No exception was found" << std::endl;
        }
        std::exit(2);
        return false;
    }};
    struct rblock {
        std::size_t seq_number;
        pgasio::array_view<const pgasio::column_meta> columns;
        pgasio::record_block block;
    };
    f5::boost_asio::channel<rblock> blocks{reactor.get_io_service(), reactor.size()};
    f5::boost_asio::channel<std::string> csj{reactor.get_io_service(), reactor.size()};

    /// Database conversation coroutine
    boost::asio::spawn(reactor.get_io_service(), [&](auto yield) {
        auto cnx = pgasio::handshake(
            pgasio::unix_domain_socket(reactor.get_io_service(), path, yield),
            user, database, yield);
        auto results = pgasio::exec(cnx, sql, yield);
        auto records = results.recordset(yield);
        auto comma = std::experimental::make_ostream_joiner(std::cout, ",");
        for ( const auto &col : records.columns ) {
            std::string escaped;
            json_string(escaped, pgasio::byte_view(
                reinterpret_cast<const unsigned char *>(col.name.data()), col.name.size()));
            *comma++ = escaped;
        }
        std::cout << std::endl;
        std::size_t block_number{};
        while ( cnx.socket.is_open() ) {
            auto block = records.next_block(yield);
            const bool good = block;
            blocks.produce({block_number++, records.columns, std::move(block)}, yield);
            if ( not good ) return;
        }
    });

    /// Workers for converting the raw data into CSJ
    for ( std::size_t t{}; t < reactor.size(); ++t ) {
        boost::asio::spawn(reactor.get_io_service(), [&](auto yield) {
            while ( true ) {
                auto batch = blocks.consume(yield);
                if ( batch.block ) {
                    std::string text;
                    text.reserve(batch.block.used_bytes());
                    for ( auto cols = batch.block.fields(); cols.size(); cols = cols.slice(batch.columns.size()) ) {
                        for ( std::size_t index{}; index < batch.columns.size(); ++index ) {
                            if ( index ) text += ',';
                           if ( cols[index].data() == nullptr ) {
                               text += "null";
                           } else {
                                switch ( batch.columns[index].field_type_oid ) {
                                case 16: // bool
                                    text += cols[index][0] == 't' ? "true" : "false";
                                    break;
                                case 21: // int2
                                case 23: // int4
                                case 20: // int8
                                case 26: // oid
                                case 700: // float4
                                case 701: // float8
                                    safe_data(text, cols[index]);
                                    break;
                                case 114: // json
                                case 1700: // numeric
                                case 1082: // date
                                case 1083: // time
                                case 1114: // timestamp without time zone
                                case 1184: // timestamp with time zone
                                case 2950: // uuid
                                case 3802: // jsonb
                                    safe_string(text, cols[index]);
                                    break;
                                default:
                                    text += "**** " +
                                        std::to_string(batch.columns[index].field_type_oid) +" **** ";
                                case 25: // text
                                case 1043: // varchar
                                    json_string(text, cols[index]);
                                }
                           }
                        }
                        text += '\n';
                    }
                    csj.produce(text, yield);
                } else {
                    csj.produce(std::string(), yield);
                }
            }
        });
    }

    /// Write the CSJ blocks out to stdout in the right order
    f5::sync s;
    boost::asio::spawn(reactor.get_io_service(), s([&](auto yield) {
        while ( true ) {
            auto chunk = csj.consume(yield);
            if ( chunk.empty() ) return;
            std::cout << chunk;
        }
    }));
    s.wait();

    blocks.close();
    csj.close();

    return 0;
}

