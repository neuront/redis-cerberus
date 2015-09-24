#ifndef __CERBERUS_BUFFER_HPP__
#define __CERBERUS_BUFFER_HPP__

#include <vector>
#include <string>

#include "utils/pointer.h"
#include "utils/mempage.hpp"
#include "syscalls/cio.h"

namespace cerb {

    void flush_string(int fd, std::string const& s);

    class Buffer {
        util::MemoryPages _buffer;
    public:
        typedef util::SharedMemPage::msize_t size_type;
        typedef util::MemPage::byte value_type;
        typedef util::MemoryPages::const_iterator const_iterator;
        typedef const_iterator iterator;

        Buffer() = default;

        static Buffer from_string(std::string s);

        Buffer(Buffer const&) = delete;

        Buffer(Buffer&& rhs)
            : _buffer(std::move(rhs._buffer))
        {}

        Buffer(iterator first, iterator last)
            : _buffer(first, last)
        {}

        Buffer& operator=(Buffer&& rhs)
        {
            _buffer = std::move(rhs._buffer);
            return *this;
        }

        iterator begin() const
        {
            return _buffer.begin();
        }

        iterator end() const
        {
            return _buffer.end();
        }

        const_iterator cbegin() const
        {
            return _buffer.begin();
        }

        const_iterator cend() const
        {
            return _buffer.end();
        }

        size_type size() const
        {
            return _buffer.size();
        }

        bool empty() const
        {
            return _buffer.empty();
        }

        void clear()
        {
            _buffer.clear();
        }

        int read(int fd);
        bool flush(int fd);
        void truncate_from_begin(iterator i);
        void buffer_ready(std::vector<cio::iovec>& iov);
        void append_from(const_iterator first, const_iterator last);
        void append_all(Buffer const& another);
        std::string to_string() const;
        bool same_as_string(std::string const& s) const;
    };

}

#endif /* __CERBERUS_BUFFER_HPP__ */
