/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>

#include <array>


namespace pgasio {


    template<typename S, typename B> inline
    void transfer(S &socker, B &buffer, std::size_t bytes, boost::asio::yield_context &yield) {
        boost::asio::async_read(socket, boost::asio::buffer(buffer.data(), buffer.size()),
            boost::asio::transfer_exactly(bytes), yield);
    };


}

