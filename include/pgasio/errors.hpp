/*
    Copyright 2017-2020, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#pragma once


#include <pgasio/memory.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>


namespace pgasio {


    /// See the Postgres documentation for the possible message values
    /// that are set here.
    /// https://www.postgresql.org/docs/current/static/protocol-error-fields.html
    class postgres_error : public std::exception {
      public:
        using messages_type = std::unordered_map<char, std::string>;
        const messages_type messages;

        postgres_error(messages_type msg) noexcept : messages(std::move(msg)) {}

        const char *what() const noexcept override {
            return "Postgres returned an error";
        }
    };


    /// Thrown when we try to read too many bytes out of a network message
    class end_of_message : public std::logic_error {
      public:
        end_of_message() : logic_error{"Ran out of bytes in network message"} {}
    };


}
