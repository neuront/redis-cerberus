#include <gtest/gtest.h>

#include "test-utils.hpp"
#include "utils/mempage.hpp"

using namespace util;

TEST(MemoryPages, CreatingAppending)
{
    MemoryPages pages;
    ASSERT_EQ(0, pages.size());
    ASSERT_TRUE(pages.empty());

    std::shared_ptr<MemPage> p(new MemPage);
    for (int i = 0; i < 20; ++i) {
        p->page[i] = MemPage::byte('a') + i;
    }
    pages.append_page(SharedMemPage(p, 20));
    ASSERT_EQ(20, pages.size());
    ASSERT_FALSE(pages.empty());

    pages.append_page(SharedMemPage(p, 5, 15));
    ASSERT_EQ(35, pages.size());
    ASSERT_FALSE(pages.empty());

    std::string s;
    for (auto i = pages.begin(); i != pages.end(); ++i) {
        s += char(*i);
    }
    ASSERT_EQ("abcdefghij"
              "klmnopqrst"
                   "fghij"
              "klmnopqrst", s);

    MemoryPages another;
    std::shared_ptr<MemPage> px(new MemPage);
    for (int i = 0; i < 10; ++i) {
        px->page[i] = MemPage::byte('0') + i;
    }
    another.append_page(SharedMemPage(px, 10));
    another.append_page(SharedMemPage(px, 3, 5));
    ASSERT_EQ(15, another.size());

    pages.append_all(another);
    ASSERT_EQ(50, pages.size());

    s.clear();
    for (auto i = pages.begin(); i != pages.end(); ++i) {
        s += char(*i);
    }
    ASSERT_EQ("abcdefghij"
              "klmnopqrst"
                   "fghij"
              "klmnopqrst"
              "0123456789"
                 "34567", s);

    pages.clear();
    ASSERT_EQ(0, pages.size());
    ASSERT_TRUE(pages.empty());
}

TEST(MemoryPages, EraseFromBeginning)
{
    std::string const M(std::string("abcde01234") * 100);
    ASSERT_EQ(1000, M.size());
    ASSERT_TRUE(M.size() < MemPage::PAGE_SIZE);

    MemoryPages pages;
    for (int i = 0; i < 20; ++i) {
        std::shared_ptr<MemPage> p(new MemPage(M));
        pages.append_page(SharedMemPage(p, M.size()));
    }
    ASSERT_EQ(M.size() * 20, pages.size());
    pages.erase_leading_pages(3, 0);
    ASSERT_EQ(M.size() * 17, pages.size());
    auto i = pages.begin();
    ASSERT_EQ('a', *i++);
    ASSERT_EQ('b', *i++);
    ASSERT_EQ('c', *i++);
    ASSERT_EQ('d', *i++);
    ASSERT_EQ('e', *i++);
    ASSERT_EQ('0', *i++);
    ASSERT_EQ('1', *i++);

    pages.erase_leading_pages(3, 7);
    ASSERT_EQ(M.size() * 14 - 7, pages.size());

    std::string const N(std::string("abcde01234") * 2);
    pages.clear();
    for (int i = 0; i < 5; ++i) {
        std::shared_ptr<MemPage> p(new MemPage(N));
        pages.append_page(SharedMemPage(p, N.size()));
    }
    pages.erase_first_bytes(5);
    ASSERT_EQ(N.size() * 5 - 5, pages.size());
    i = pages.begin();
    ASSERT_EQ('0', *i++);
    ASSERT_EQ('1', *i++);
    ASSERT_EQ('2', *i++);
    ASSERT_EQ('3', *i++);
    ASSERT_EQ('4', *i++);
    ASSERT_EQ('a', *i++);
    ASSERT_EQ('b', *i++);

    pages.erase_first_bytes(14);
    ASSERT_EQ(N.size() * 4 + 1, pages.size());
    i = pages.begin();
    ASSERT_EQ('4', *i++);
    ASSERT_EQ('a', *i++);
    ASSERT_EQ('b', *i++);
    ASSERT_EQ('c', *i++);

    pages.erase_first_bytes(1);
    ASSERT_EQ(N.size() * 4, pages.size());
    i = pages.begin();
    ASSERT_EQ('a', *i++);
    ASSERT_EQ('b', *i++);
    ASSERT_EQ('c', *i++);
    ASSERT_EQ('d', *i++);

    pages.erase_first_bytes(N.size() + 8);
    ASSERT_EQ(N.size() * 3 - 8, pages.size());
    i = pages.begin();
    ASSERT_EQ('3', *i++);
    ASSERT_EQ('4', *i++);
    ASSERT_EQ('a', *i++);
    ASSERT_EQ('b', *i++);

    pages.erase_first_bytes(N.size() * 3 - 8);
    ASSERT_EQ(0, pages.size());
    ASSERT_TRUE(pages.empty());
    i = pages.begin();
}
