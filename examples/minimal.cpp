/**
    Copyright 2017-2018, Kirit Sælensminde. <https://kirit.com/pgasio/>
*/
/// # minimal
///
/// Minimal example showing how to use the library.


#include <cstdlib> // for std::getenv
#include <iostream>
#include <string>

#include <pgasio/buffered.hpp>
#include <pgasio/query.hpp>

#include <boost/asio/io_service.hpp>


using namespace std::string_literals;


int main() {
    /// The parameters we need to use.
    const char *user = std::getenv("LOGNAME");
    const char *database = nullptr;
    const char *path = "/var/run/postgresql/.s.PGSQL.5432";
    const char *sql = "SELECT 42";

    /// We need a container for the coroutines
    boost::asio::io_service ios;
    /// Spawn a coroutine to perform our database operations with
    boost::asio::spawn(ios, [&](auto yield) {
        /// Open the connection to postgres. This creates a buffered unix
        /// domain socket. The buffering makes a huge difference to
        /// throughput, especially where rows don't contain much data
        auto cnx = pgasio::handshake(
                pgasio::make_buffered(
                        pgasio::unix_domain_socket(ios, path, yield)),
                user, database, yield);
        std::cout << "Connected to " << path << " as " << user << std::endl;

        /// Submit the SQL to Postgres. We get a result that is a holder
        /// for the recordsets
        auto results = pgasio::query(cnx, sql, yield);
        auto recordset = results.recordset(yield);

        /// The recordset contains meta-data describing the column types
        std::cout << "Data type for first column is "
                  << recordset.columns()[0].field_type_oid
                  << " (23 means a 32 bit integer)" << std::endl;

        /// From the recordset we can get the first block of data
        auto block = recordset.next_block(yield);

        /// From the block we can get the first column for the first row.
        /// Note that the number is returned as text. You will need to
        /// parse it to use it as a number.
        auto data = block.fields()[0];
        std::cout << "Data returned is: ";
        for (char c : data) std::cout << c;
        std::cout << std::endl;
    });
    /// Finally run the coroutine in this thread
    ios.run();

    return 0;
}
