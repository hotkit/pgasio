/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#ifndef PGASIO_MEMORY_HPP
#define PGASIO_MEMORY_HPP
#pragma once


#include <array>
#include  <vector>


namespace pgasio {


    /// Basic view of contiguous blocks of some type. Two views
    /// are the same only if they point to the same underlying memory.
    template<typename V>
    class array_view final {
        const V *m_data;
        std::size_t m_size;
    public:
        /// The underlying pointer type
        using pointer_type = const V*;

        /// Default construct an empty buffer
        array_view()
        : m_data(nullptr), m_size(0u) {
        }

        /// Construct from a vector
        array_view(const std::vector<V> &v)
        : m_data(v.data()), m_size(v.size()) {
        }
        template<std::size_t N>
        array_view(const std::array<V, N> &v)
        : m_data(v.data()), m_size(N) {
        }
        /// Construct from an array
        array_view(const V *a, std::size_t items)
        : m_data(a), m_size(items) {
        }

        /// The start of the data array
        pointer_type data() const {
            return m_data;
        }
        /// The number of items in the array
        std::size_t size() const {
            return m_size;
        }

        /// Return a slice of this array
        array_view slice(std::size_t start) {
            return array_view(m_data + start, m_size - start);
        }
        array_view slice(std::size_t start, std::size_t items) {
            return array_view(m_data + start, items);
        }

        /// Index into the arraay
        const V &operator [] (std::size_t index) {
            return data()[index];
        }

        /// Constant iterator
        using const_iterator = pointer_type;
        /// Start iterator
        const_iterator begin() const {
            return data();
        }
        /// End iterator
        const_iterator end() const {
            return data() + size();
        }

        /// Reverse const iterator
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        /// Start at the end
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }
        /// End at the beginning
        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }
    };


    /// View into memory.
    using byte_view = array_view<unsigned char>;


}


#endif

