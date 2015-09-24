#include "utils/string.h"
#include "test-utils.hpp"
#include "mock-io.hpp"
#include "core/buffer.hpp"
#include "core/fdutil.hpp"

using namespace cerb;

typedef BufferTestBase BufferTest;

TEST_F(BufferTest, AsString)
{
    Buffer buffer(Buffer::from_string("need a light"));
    ASSERT_EQ(12, buffer.size());
    ASSERT_EQ(std::string("need a light"), buffer.to_string());

    Buffer cuffer(Buffer::from_string("ghost reporting"));
    ASSERT_EQ(15, cuffer.size());
    ASSERT_EQ(std::string("ghost reporting"), cuffer.to_string());

    buffer.append_from(cuffer.begin(), cuffer.end());
    ASSERT_EQ(27, buffer.size());
    ASSERT_EQ(std::string("need a lightghost reporting"), buffer.to_string());
}

TEST_F(BufferTest, IO)
{
    Buffer buffer;
    int n = buffer.read(-1);
    ASSERT_EQ(0, n);
    ASSERT_TRUE(buffer.empty());

    BufferTest::io_obj->read_buffer.push_back("the quick brown fox jumps over");
    BufferTest::io_obj->read_buffer.push_back(" a lazy dog");
    int buf_len = BufferTest::io_obj->read_buffer[0].size()
                + BufferTest::io_obj->read_buffer[1].size();

    n = buffer.read(-1);
    ASSERT_EQ(buf_len, n);
    ASSERT_EQ(std::string("the quick brown fox jumps over a lazy dog"), buffer.to_string());
}

TEST_F(BufferTest, WriteVectorSimple)
{
    Buffer head(Buffer::from_string("0123456789abcdefghij"));
    Buffer body(Buffer::from_string("QWEASDZXC+RTYFGHVBN-"));
    Buffer tail(Buffer::from_string("!@#$%^&*()ABCDEFGHIJ"));

    {
        BufferTest::io_obj->clear();
        Buffer bufset;
        bufset.append_all(head);
        bufset.append_all(body);
        bufset.append_all(tail);

        bool w = bufset.flush(0);
        ASSERT_TRUE(w);
        ASSERT_TRUE(bufset.empty());

        ASSERT_EQ(3, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ(body.to_string(), BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ(tail.to_string(), BufferTest::io_obj->write_buffer[2]);
    }

    {
        BufferTest::io_obj->clear();
        BufferTest::io_obj->writing_sizes.push_back(50);

        Buffer bufset;
        bufset.append_all(head);
        bufset.append_all(body);
        bufset.append_all(tail);

        bool w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(3, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ(body.to_string(), BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("!@#$%^&*()", BufferTest::io_obj->write_buffer[2]);

        w = bufset.flush(0);
        ASSERT_TRUE(w);
        ASSERT_TRUE(bufset.empty());

        ASSERT_EQ(4, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ(body.to_string(), BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("!@#$%^&*()", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("ABCDEFGHIJ", BufferTest::io_obj->write_buffer[3]);
    }

    {
        BufferTest::io_obj->clear();
        BufferTest::io_obj->writing_sizes.push_back(30);
        BufferTest::io_obj->writing_sizes.push_back(17);

        Buffer bufset;
        bufset.append_all(head);
        bufset.append_all(body);
        bufset.append_all(tail);

        bool w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);

        w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(4, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&", BufferTest::io_obj->write_buffer[3]);

        w = bufset.flush(0);
        ASSERT_TRUE(w);
        ASSERT_TRUE(bufset.empty());

        ASSERT_EQ(5, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&", BufferTest::io_obj->write_buffer[3]);
        ASSERT_EQ("*()ABCDEFGHIJ", BufferTest::io_obj->write_buffer[4]);
    }

    {
        BufferTest::io_obj->clear();
        BufferTest::io_obj->writing_sizes.push_back(30);
        BufferTest::io_obj->writing_sizes.push_back(20);

        Buffer bufset;
        bufset.append_all(head);
        bufset.append_all(body);
        bufset.append_all(tail);

        bool w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);

        w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(4, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&*()", BufferTest::io_obj->write_buffer[3]);

        w = bufset.flush(0);
        ASSERT_TRUE(w);
        ASSERT_TRUE(bufset.empty());

        ASSERT_EQ(5, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&*()", BufferTest::io_obj->write_buffer[3]);
        ASSERT_EQ("ABCDEFGHIJ", BufferTest::io_obj->write_buffer[4]);
    }

    {
        BufferTest::io_obj->clear();
        BufferTest::io_obj->writing_sizes.push_back(30);
        BufferTest::io_obj->writing_sizes.push_back(23);

        Buffer bufset;
        bufset.append_all(head);
        bufset.append_all(body);
        bufset.append_all(tail);

        bool w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);

        w = bufset.flush(0);
        ASSERT_FALSE(w);
        ASSERT_FALSE(bufset.empty());

        ASSERT_EQ(4, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&*()ABC", BufferTest::io_obj->write_buffer[3]);

        w = bufset.flush(0);
        ASSERT_TRUE(w);
        ASSERT_TRUE(bufset.empty());

        ASSERT_EQ(5, BufferTest::io_obj->write_buffer.size());
        ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
        ASSERT_EQ("QWEASDZXC+", BufferTest::io_obj->write_buffer[1]);
        ASSERT_EQ("RTYFGHVBN-", BufferTest::io_obj->write_buffer[2]);
        ASSERT_EQ("!@#$%^&*()ABC", BufferTest::io_obj->write_buffer[3]);
        ASSERT_EQ("DEFGHIJ", BufferTest::io_obj->write_buffer[4]);
    }
}

TEST_F(BufferTest, PartiallyWrite)
{
    Buffer head(Buffer::from_string("0123456789!@#$%^&*()"));
    Buffer body(Buffer::from_string("QWERTYUIOPqwertyuiop"));
    Buffer tail(Buffer::from_string("ABCDEFGHIJabcdefghij"));

    // exactly 20 bytes

    BufferTest::io_obj->writing_sizes.push_back(20);
    BufferTest::io_obj->writing_sizes.push_back(-1);
    BufferTest::io_obj->writing_sizes.push_back(20);
    BufferTest::io_obj->writing_sizes.push_back(-1);
    BufferTest::io_obj->writing_sizes.push_back(20);
    BufferTest::io_obj->writing_sizes.push_back(-1);

    Buffer buf;
    buf.append_all(head);
    buf.append_all(body);
    buf.append_all(tail);

    bool w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ(head.to_string(), BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(40, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ(body.to_string(), BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(20, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_TRUE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ(tail.to_string(), BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(0, buf.size());
    ASSERT_TRUE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();
    BufferTest::io_obj->writing_sizes.clear();

    // 1 byte less

    BufferTest::io_obj->writing_sizes.push_back(19);
    BufferTest::io_obj->writing_sizes.push_back(19);
    BufferTest::io_obj->writing_sizes.push_back(19);
    BufferTest::io_obj->writing_sizes.push_back(19);

    buf.append_all(head);
    buf.append_all(body);
    buf.append_all(tail);

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("0123456789!@#$%^&*(", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(41, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ(")", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ("QWERTYUIOPqwertyui", BufferTest::io_obj->write_buffer[1]);
    ASSERT_EQ(22, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("op", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ("ABCDEFGHIJabcdefg", BufferTest::io_obj->write_buffer[1]);
    ASSERT_EQ(3, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_TRUE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("hij", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(0, buf.size());
    ASSERT_TRUE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();
    BufferTest::io_obj->writing_sizes.clear();

    // 1 byte more

    BufferTest::io_obj->writing_sizes.push_back(21);
    BufferTest::io_obj->writing_sizes.push_back(21);
    BufferTest::io_obj->writing_sizes.push_back(21);

    buf.append_all(head);
    buf.append_all(body);
    buf.append_all(tail);

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("0123456789!@#$%^&*()", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ("Q", BufferTest::io_obj->write_buffer[1]);
    ASSERT_EQ(39, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_FALSE(w);
    ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("WERTYUIOPqwertyuiop", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ("AB", BufferTest::io_obj->write_buffer[1]);
    ASSERT_EQ(18, buf.size());
    ASSERT_FALSE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();

    w = buf.flush(0);
    ASSERT_TRUE(w);
    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("CDEFGHIJabcdefghij", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ(0, buf.size());
    ASSERT_TRUE(buf.empty());
    BufferTest::io_obj->write_buffer.clear();
    BufferTest::io_obj->writing_sizes.clear();
}

TEST_F(BufferTest, WriteSinglePieceMultipleTimes)
{
    Buffer x(Buffer::from_string("0123456789abcdefghij"));
    Buffer y(Buffer::from_string("QWEASDZXC+RTYFGHVBN-"));

    BufferTest::io_obj->writing_sizes.push_back(1);
    BufferTest::io_obj->writing_sizes.push_back(2);
    BufferTest::io_obj->writing_sizes.push_back(3);
    BufferTest::io_obj->writing_sizes.push_back(4);
    BufferTest::io_obj->writing_sizes.push_back(5);
    BufferTest::io_obj->writing_sizes.push_back(6);
    BufferTest::io_obj->writing_sizes.push_back(2);

    Buffer bufset;
    bufset.append_all(x);
    bufset.append_all(y);

    bool w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(1, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("0", BufferTest::io_obj->write_buffer[0]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(2, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("0", BufferTest::io_obj->write_buffer[0]);
    ASSERT_EQ("12", BufferTest::io_obj->write_buffer[1]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(3, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("12", BufferTest::io_obj->write_buffer[1]);
    ASSERT_EQ("345", BufferTest::io_obj->write_buffer[2]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(4, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("345", BufferTest::io_obj->write_buffer[2]);
    ASSERT_EQ("6789", BufferTest::io_obj->write_buffer[3]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(5, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("6789", BufferTest::io_obj->write_buffer[3]);
    ASSERT_EQ("abcde", BufferTest::io_obj->write_buffer[4]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(7, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("abcde", BufferTest::io_obj->write_buffer[4]);
    ASSERT_EQ("fghij", BufferTest::io_obj->write_buffer[5]);
    ASSERT_EQ("Q", BufferTest::io_obj->write_buffer[6]);

    w = bufset.flush(0);
    ASSERT_FALSE(w);
    ASSERT_FALSE(bufset.empty());

    ASSERT_EQ(8, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("fghij", BufferTest::io_obj->write_buffer[5]);
    ASSERT_EQ("Q", BufferTest::io_obj->write_buffer[6]);
    ASSERT_EQ("WE", BufferTest::io_obj->write_buffer[7]);

    w = bufset.flush(0);
    ASSERT_TRUE(w);
    ASSERT_TRUE(bufset.empty());

    ASSERT_EQ(9, BufferTest::io_obj->write_buffer.size());
    ASSERT_EQ("ASDZXC+RTYFGHVBN-", BufferTest::io_obj->write_buffer[8]);
}

TEST_F(BufferTest, WriteVectorLargeBuffer)
{
    int const WRITEV_SIZE = util::MemPage::PAGE_SIZE / 4;
    int fill_x = WRITEV_SIZE * 2;
    int fill_y = WRITEV_SIZE * 3;
    int fill_z = WRITEV_SIZE * 4;

    Buffer b(Buffer::from_string("begin"));
    Buffer x(Buffer::from_string(std::string(fill_x, 'x')));
    Buffer y(Buffer::from_string(std::string(fill_y, 'y')));
    Buffer z(Buffer::from_string(std::string(fill_z, 'z')));
    Buffer a(Buffer::from_string("abc"));
    Buffer m(Buffer::from_string(std::string(WRITEV_SIZE, 'm')));
    Buffer u(Buffer::from_string("uvw"));

    Buffer bufset;
    bufset.append_all(b);
    bufset.append_all(x);
    bufset.append_all(y);
    bufset.append_all(z);
    bufset.append_all(a);
    bufset.append_all(m);
    bufset.append_all(u);

    bool w = bufset.flush(0);
    ASSERT_TRUE(w);
    ASSERT_TRUE(bufset.empty());

    ASSERT_EQ(7, BufferTest::io_obj->write_buffer.size());

    ASSERT_EQ(b.size(), BufferTest::io_obj->write_buffer[0].size());
    ASSERT_EQ("begin", BufferTest::io_obj->write_buffer[0]);

    ASSERT_EQ(fill_x, BufferTest::io_obj->write_buffer[1].size());
    for (unsigned i = 0; i < BufferTest::io_obj->write_buffer[1].size(); ++i) {
        ASSERT_EQ('x', BufferTest::io_obj->write_buffer[1][i]) << " at " << i;
    }

    ASSERT_EQ(fill_y, BufferTest::io_obj->write_buffer[2].size());
    for (unsigned i = b.size(); i < BufferTest::io_obj->write_buffer[2].size(); ++i) {
        ASSERT_EQ('y', BufferTest::io_obj->write_buffer[2][i]) << " at " << i;
    }

    ASSERT_EQ(fill_z, BufferTest::io_obj->write_buffer[3].size());
    for (unsigned i = b.size(); i < BufferTest::io_obj->write_buffer[3].size(); ++i) {
        ASSERT_EQ('z', BufferTest::io_obj->write_buffer[3][i]) << " at " << i;
    }

    ASSERT_EQ(a.size(), BufferTest::io_obj->write_buffer[4].size());
    ASSERT_EQ("abc", BufferTest::io_obj->write_buffer[4]);

    ASSERT_EQ(WRITEV_SIZE, BufferTest::io_obj->write_buffer[5].size());
    for (unsigned i = b.size(); i < BufferTest::io_obj->write_buffer[5].size(); ++i) {
        ASSERT_EQ('m', BufferTest::io_obj->write_buffer[5][i]) << " at " << i;
    }

    ASSERT_EQ(u.size(), BufferTest::io_obj->write_buffer[6].size());
    ASSERT_EQ("uvw", BufferTest::io_obj->write_buffer[6]);
}

TEST_F(BufferTest, Write50KBuffers)
{
    ASSERT_EQ(10, (std::string("ab") * 5).size());
    int const SIZE = 50000;
    std::vector<Buffer> storage;
    Buffer bufset;
    for (int i = 0; i < SIZE; ++i) {
        storage.push_back(Buffer::from_string(
            ("VALUE:" + util::str(100000000LL + i) + '$') * 40));
    }
    for (int i = 0; i < SIZE; ++i) {
        bufset.append_all(storage[i]);
    }

    bool w = bufset.flush(0);
    ASSERT_TRUE(w);

    ASSERT_EQ(SIZE, BufferTest::io_obj->write_buffer.size());
    for (int i = 0; i < SIZE; ++i) {
        ASSERT_EQ(("VALUE:" + util::str(100000000LL + i) + '$') * 40,
                  BufferTest::io_obj->write_buffer[i])
            << " at " << i;
    }
}
