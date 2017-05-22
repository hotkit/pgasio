/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#include <cstdlib>
#include <fstream>
#include <iostream>

#include <pgasio/exec.hpp>


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
            sql.emplace_back(content);
        }
    }

    boost::asio::io_service ios;
    boost::asio::spawn(ios, [&](auto yield) {
        auto cnx = pgasio::handshake(
            pgasio::unix_domain_socket(ios, path, yield),
            user, database, yield);

        for ( const auto &commands : sql ) {
            std::cout << commands.substr(0, 60) << "...." << std::endl;
        }
    });
    ios.run();

    return 0;
}

