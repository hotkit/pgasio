/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pgasio/buffered.hpp>
#include <pgasio/query.hpp>


/// Store statistics about the column data
struct col_stats {
    pgasio::column_meta meta;

    std::size_t nulls{}, not_nulls{};
    std::size_t size{};

    void operator () (pgasio::byte_view);
};


/// Store statistics about a single recordset we get back
struct stats {
    std::chrono::time_point<std::chrono::steady_clock> started
        = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> ended{};
    std::size_t blocks = 0;
    std::size_t rows = 0;

    std::vector<col_stats> cols;

    void operator += (const stats &s) {
        blocks += s.blocks;
        rows += s.rows;
    }

    void done() {
        ended = std::chrono::steady_clock::now();
    }
};
template<class Ch, class Tr, class... Args>
auto operator << (std::basic_ostream<Ch, Tr> &os, const stats &s)
    -> std::basic_ostream<Ch, Tr>&;


/// Store the statistics about the commands (potentially multiple recordsets)
struct cmd_stats : public stats {
    std::string command;
    std::vector<stats> recordsets;
    cmd_stats(std::string cmd)
    : command(std::move(cmd)) {
    }
};
template<class Ch, class Tr, class... Args>
auto operator << (std::basic_ostream<Ch, Tr> &, const cmd_stats &)
    -> std::basic_ostream<Ch, Tr>&;


int main(int argc, char *argv[]) {
    std::cerr << "Recordset statistics" << std::endl;

    /// The parameters we need to use.
    const char *user = std::getenv("LOGNAME");
    const char *database = nullptr;
    const char *path = "/var/run/postgresql/.s.PGSQL.5432";
    std::vector<std::string> sql;

    /// Go through the command line and pull the details out
    for ( auto a{1}; a < argc; ++a ) {
        using namespace std::string_literals;
        auto read_opt = [&](char opt) {
            if ( ++a >= argc ) throw std::runtime_error("Missing option after -"s + opt);
            return argv[a];
        };
        if ( argv[a] == "-c"s ) {
            sql.emplace_back(read_opt('c'));
        } else if ( argv[a] == "-d"s ) {
            database = read_opt('d');
        } else if ( argv[a] == "-h"s ) {
            path = read_opt('h');
        } else if ( argv[a] == "-U"s ) {
            user = read_opt('U');
        } else if ( argv[a][0] == '-' ) {
            std::cerr << "Unknown command line option: "s + argv[a][0] << std::endl;
            return 2;
        } else {
            std::ifstream file(argv[a]);
            std::string content;
            for ( char c{}; file.get(c); content += c );
            if ( content.empty() ) {
                std::cerr << "File " << argv[a] << " was empty, or could not be read" << std::endl;
            } else {
                sql.emplace_back(content);
            }
        }
    }

    /// The statistics we want to capture and display
    stats overall;
    std::vector<cmd_stats> individual;
    /// The io_service acts as a container within which coroutines run.
    boost::asio::io_service ios;
    /// For each command spawn a coroutine to execute it
    boost::asio::spawn(ios, [&](auto yield) {
        try {
            auto cnx = pgasio::handshake(
                pgasio::make_buffered(pgasio::unix_domain_socket(ios, path, yield)),
                user, database, yield);

            for ( const auto &commands : sql ) {
                cmd_stats cmd_stat{commands};
                auto results = pgasio::query(cnx, commands, yield);
                while ( auto rs = results.recordset(yield) ) {
                    stats rs_stats;
                    for ( auto &&col : rs.columns() ) {
                        rs_stats.cols.push_back(col_stats{col});
                    }
                    while ( auto block = rs.next_block(yield) ) {
                        ++rs_stats.blocks;
                        for ( auto fields = block.fields(); fields.size();
                            fields = fields.slice(rs.columns().size()) )
                        {
                            for ( std::size_t col{}; col < rs_stats.cols.size(); ++col ) {
                                rs_stats.cols[col](fields[col]);
                            }
                            ++rs_stats.rows;
                        }
                    }
                    rs_stats.done();
                    cmd_stat += rs_stats;
                    cmd_stat.recordsets.emplace_back(std::move(rs_stats));
                }
                cmd_stat.done();
                overall += cmd_stat;
                individual.emplace_back(std::move(cmd_stat));
            }
        } catch ( pgasio::postgres_error &e ) {
            std::cerr << "Postgres error: " << e.what() << std::endl;
            std::exit(1);
        } catch ( std::exception &e ) {
            std::cerr << "std::exception: " << e.what() << std::endl;
            std::exit(2);
        }
    });
    ios.run();
    overall.done();
    for ( const auto &stat : individual ) {
        std::cout << stat << std::endl;
    }
    std::cout << "Overall\n" << overall << std::endl;

    return 0;
}


/// Implementations of statistics gathering
void col_stats::operator () (pgasio::byte_view field) {
    if ( field.data() == nullptr ) {
        ++nulls;
    } else {
        ++not_nulls;
        size += field.size();
    }
}


/// Display the statistics we capture
template<class Ch, class Tr, class... Args>
auto operator << (std::basic_ostream<Ch, Tr>& os, const col_stats &s)
    -> std::basic_ostream<Ch, Tr>&
{
    const auto rows = s.nulls + s.not_nulls;
    os << "  " << s.meta.name << ": oid=" << s.meta.field_type_oid
        << "\n    null: " << s.nulls << '/' << rows << " not null: " << s.not_nulls
        << "\n    size: total=" << s.size;
    if ( rows ) os << " mean=" << (double(s.size)/rows);
    return os;
}
template<class Ch, class Tr, class... Args>
auto operator << (std::basic_ostream<Ch, Tr> &os, const stats &s)
    -> std::basic_ostream<Ch, Tr>&
{
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(s.ended - s.started);
    auto ms = time.count();
    os << "Time: " <<ms << "ms\nRows: " << s.rows
        << " (blocks= " << s.rows << ')';
    if ( ms ) os << "\nRows per second: " << (1000 * s.rows / ms);
    for ( auto &&cs : s.cols ) os << '\n' << cs;
    return os;
}
template<class Ch, class Tr, class... Args>
auto operator << (std::basic_ostream<Ch, Tr> &os, const cmd_stats &s)
    -> std::basic_ostream<Ch, Tr>&
{
    os<< "-- Command --\n" << s.command<< '\n' << "-------------" << '\n';
    std::size_t rs_number{};
    for ( const auto &rs : s.recordsets ) {
        os << "\nRecordset " << ++rs_number << '\n' << rs;
    }
    os << "\n\nCommand overall:\n" << static_cast<const stats &>(s) << "\n\n";
    return os;
}

