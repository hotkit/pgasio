/*
    Copyright 2017, Kirit Sælensminde. http://www.kirit.com/pgasio/
*/


#ifndef PGASIO_MEMORY_HPP
#define PGASIO_MEMORY_HPP
#pragma once


#include <array>
#include <cassert>
#include  <vector>


namespace pgasio {


    /// Basic view of contiguous blocks of some type. Two views
    /// are the same only if they point to the same underlying memory.
    template<typename V>
    class array_view final {
        V *m_data;
        std::size_t m_size;
    public:
        /// The value type
        using value_type = V;
        /// The underlying pointer type
        using pointer_type = V*;

        /// Default construct an empty buffer
        array_view()
        : m_data(nullptr), m_size(0u) {
        }

        /// Construct from a vector
        array_view(std::vector<value_type> &v)
        : m_data(v.data()), m_size(v.size()) {
        }
        template<typename T>
        array_view(const std::vector<T> &v)
        : m_data(v.data()), m_size(v.size()) {
        }
        template<typename T, std::size_t N>
        array_view(std::array<T, N> &v)
        : m_data(v.data()), m_size(N) {
        }
        /// Construct from an array
        array_view(pointer_type a, std::size_t items)
        : m_data(a), m_size(items) {
        }

        /// The start of the data array
        pointer_type data() {
            return m_data;
        }
        const pointer_type data() const {
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
        using const_iterator = const pointer_type;
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


    /// View into constant memory.
    using byte_view = array_view<const unsigned char>;
    /// Ram alterable memory content
    using raw_memory = array_view<unsigned char>;


    /// A slab of memory that can be chopped up without regard to
    /// alignment issues.
    class unaligned_slab {
        std::vector<unsigned char> buffer;
        std::size_t base;

    public:
        /// Create a slab for the requested number of bytes
        unaligned_slab(std::size_t bytes)
        : buffer(bytes), base{} {
        }

        /// Not copyable
        unaligned_slab(const unaligned_slab &) = delete;
        unaligned_slab &operator = (const unaligned_slab &) = delete;

        /// How many bytes are still left in this slab
        std::size_t remaining() const {
            return buffer.size() - base;
        }

        /// Allocate a chunk of the slab of the requested size and returns
        /// a pointer to it
        raw_memory allocate(std::size_t bytes) {
            assert(bytes <= remaining());
            auto allocated = buffer.data() + base;
            base += bytes;
            return raw_memory(allocated, bytes);
        }
    };


}


#endif

