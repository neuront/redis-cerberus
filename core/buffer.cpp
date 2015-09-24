#include <climits>
#include <algorithm>

#include "buffer.hpp"
#include "globals.hpp"
#include "except/exceptions.hpp"
#include "utils/logging.hpp"

using namespace cerb;
using util::MemPage;
using util::SharedMemPage;
using util::MemoryPages;

static void on_error(std::string const& message)
{
    if (errno == EAGAIN) {
        return;
    }
    if (errno == ETIMEDOUT || errno == ECONNABORTED || errno == ECONNREFUSED
            || errno == ECONNRESET || errno == EHOSTUNREACH || errno == EIO)
    {
        throw IOError(message, errno);
    }
    throw SystemError(message, errno);
}

void cerb::flush_string(int fd, std::string const& s)
{
    std::string::size_type n = 0;
    while (n < s.size()) {
        int nwrite = cio::write(fd, s.data() + n, s.size() - n);
        if (nwrite == -1) {
            on_error("buffer write");
            continue;
        }
        n += nwrite;
    }
}

static void page_del(MemPage* p)
{
    cerb_global::page_pool.destroy(p);
}

static std::shared_ptr<MemPage> alloc_page()
{
    auto* p = cerb_global::page_pool.create();
    return std::shared_ptr<MemPage>(p, page_del);
}

static std::shared_ptr<MemPage> alloc_page(std::string s)
{
    auto* p = cerb_global::page_pool.create(std::move(s));
    return std::shared_ptr<MemPage>(p, page_del);
}

static SharedMemPage make_page(std::string s)
{
    auto sz = s.size();
    return SharedMemPage(alloc_page(std::move(s)), sz);
}

Buffer Buffer::from_string(std::string s)
{
    Buffer b;
    b._buffer.append_page(::make_page(std::move(s)));
    return std::move(b);
}

static int read_to_buffer(int fd, byte* page, msize_t buf_len)
{
    msize_t n = 0;
    int nread;
    while (n < buf_len && 0 < (nread = cio::read(fd, page + n, buf_len - n))) {
        n += nread;
    }
    if (nread == -1) {
        on_error("buffer read");
    }
    return n;
}

int Buffer::read(int fd)
{
    int n = 0, nread;
    for (auto page = ::alloc_page();
         0 < (nread = ::read_to_buffer(fd, page->page, MemPage::PAGE_SIZE));
         page = ::alloc_page())
    {
        n += nread;
        this->_buffer.append_page(SharedMemPage(page, nread));
        if (nread != MemPage::PAGE_SIZE) {
            break;
        }
    }
    return n;
}

void Buffer::truncate_from_begin(iterator i)
{
    this->_buffer.erase_from_begin(i);
}

void Buffer::buffer_ready(std::vector<cio::iovec>& iov)
{
    for (auto page_it = this->_buffer.pages_begin();
         page_it != this->_buffer.pages_end();
         ++page_it)
    {
        cio::iovec v = {page_it->page(), size_t(page_it->size)};
        LOG(DEBUG) << "Push iov " << reinterpret_cast<void*>(page_it->page()) << ' ' << page_it->size;
        iov.push_back(v);
    }
}

void Buffer::append_from(const_iterator first, const_iterator last)
{
    this->_buffer.append_range(first, last);
}

void Buffer::append_all(Buffer const& another)
{
    this->_buffer.append_all(another._buffer);
}

std::string Buffer::to_string() const
{
    std::string s;
    for (auto page_it = this->_buffer.pages_begin();
         page_it != this->_buffer.pages_end();
         ++page_it)
    {
        s.insert(s.end(), page_it->page(), page_it->page() + page_it->size);
    }
    return std::move(s);
}

bool Buffer::same_as_string(std::string const& s) const
{
    if (this->size() != s.size()) {
        return false;
    }
    return this->to_string() == s;
}

static msize_t write_single(int fd, byte const* buf, msize_t buf_len)
{
    msize_t offset = 0;
    while (offset < buf_len) {
        ssize_t nwritten = cio::write(fd, buf + offset, buf_len - offset);
        if (nwritten == -1) {
            on_error("buffer write");
            return offset;
        }
        LOG(DEBUG) << "Write to " << fd << " : " << nwritten << " bytes written";
        offset += nwritten;
    }
    return offset;
}

static msize_t write_vec(int fd, int iovcnt, cio::iovec* iov, ssize_t total)
{
    if (1 == iovcnt) {
        return write_single(fd, reinterpret_cast<byte*>(iov->iov_base),
                            iov->iov_len);
    }

    LOG(DEBUG) << "*writev to " << fd << " iovcnt=" << iovcnt << " total bytes=" << total;
    int written_iov = 0;
    ssize_t nwritten;
    msize_t w = 0;
    while (total != (nwritten = cio::writev(fd, iov + written_iov, iovcnt - written_iov))) {
        if (nwritten == 0) {
            return w;
        }
        if (nwritten == -1) {
            on_error("buffer writev");
            return w;
        }
        LOG(DEBUG) << "*writev partial: " << nwritten << " / " << total;
        w += nwritten;
        total -= nwritten;
        while (iov[written_iov].iov_len <= size_t(nwritten)) {
            nwritten -= iov[written_iov].iov_len;
            ++written_iov;
        }
        iov[written_iov].iov_base = reinterpret_cast<byte*>(iov[written_iov].iov_base) + nwritten;
        iov[written_iov].iov_len -= nwritten;
    }
    return w + total;
}

static msize_t const WRITEV_MAX_SIZE = 2 * 1024 * 1024;

template <typename PagesIterator>
static std::pair<int, msize_t> next_group_to_write(PagesIterator i, PagesIterator end,
                                                   cio::iovec* vec)
{
    vec[0].iov_base = i->page();
    vec[0].iov_len = i->size;
    msize_t bulk_write_size = i->size;
    ++i;
    int count = 1;
    for (; i != end
            && count < IOV_MAX
            && bulk_write_size + i->size <= WRITEV_MAX_SIZE;
         ++i, ++count)
    {
        vec[count].iov_base = i->page();
        vec[count].iov_len = i->size;
        bulk_write_size += i->size;
    }
    return std::make_pair(count, bulk_write_size);
}

bool Buffer::flush(int fd)
{
    cio::iovec vec[IOV_MAX];
    while (!this->_buffer.empty()) {
        auto iovcnt_write_sz = ::next_group_to_write(this->_buffer.pages_begin(),
                                                     this->_buffer.pages_end(), vec);
        int& iovcnt = iovcnt_write_sz.first;
        msize_t& total = iovcnt_write_sz.second;
        msize_t written = ::write_vec(fd, iovcnt, vec, total);
        this->_buffer.erase_first_bytes(written);
        if (total != written) {
            return false;
        }
    }
    return true;
}
