A Redis cluster proxy.

Build
===

Requirements:

* `make` and `cmake`
* UNIX-like system with `SO_REUSEPORT | SO_REUSEADDR` support
* `epoll` support
* pthread
* C++ compiler & lib with C++11 features, like g++ 4.8 or clang++ 3.2 (NOTE: install clang++ 3.2 on CentOS 6.5 won't compile because clang uses header files from gcc, which is version 4.4 without C++11 support)
* Google Test (for test)

To build, just

    make

turn on all debug logs

    make MODE=debug

or compile with g艹

    make COMPILER=g++

To link libstdc++ statically, use

    make STATIC_LINK=1

to run test (just cover message parsing parts)

    make runtest

run test with valgrind checking

    make runtest CHECK_MEM=1

Run
===

    cerberus CONFIG_FILE [ARGS]

The first argument is path of a configuration file, then optional arguments. Those specifies

* bind / `-b` : (integer) local port to listen; could also specified
* node / `-n` : (address, optional) one of active node in a cluster; format should be *host:port*; could also set after cerberus launched, via the `SETREMOTES` command, see it below
* thread / `-t` : (integer) number of threads
* read-slave / `r` : (optional, default off) set to "1" to turn on read slave mode. A proxy in read-slave mode won't support writing commands like `SET`, `INCR`, `PUBLISH`, and it would select slave nodes for reading commands if possible.  For more information please read [here (CN)](https://github.com/HunanTV/redis-cerberus/wiki/%E8%AF%BB%E5%86%99%E5%88%86%E7%A6%BB).

The option set via ARGS would override it in the configuration file. For example

    cerberus example.conf -t 8

set the program to 8 threads.

Commands in Particular
===

Restricted Commands Bypass
---

* `MGET` : execute multiple `GET`s
* `MSET` : execute multiple `SET`s
* `DEL` : execute multiple `DEL`s
* `RENAME` : if source and destination are not in the same slot, execute a `GET`-`SET`-`DEL` sequence without atomicity
* `BLPOP` / `BRPOP`: one list limited; might return nil value before timeout [See detail (CN)](https://github.com/HunanTV/redis-cerberus/wiki/BLPOP-And-BRPOP)

Extra Commands
---

* `PROXY` / `INFO`: show proxy information, including threads count, clients counts, commands statistics, and remote redis servers
* `KEYSINSLOT slot count`: list keys in a specified slot, same as `CLUSTER GETKEYSINSLOT slot count`
* `UPDATESLOTMAP`: notify each thread to update slot map after the next operation
* `SETREMOTES host port host port ...`: reset redis server addresses to arguments, and update slot map after that

Not Implemented
---

* keys: `KEYS`, `MIGRATE`, `MOVE`, `OBJECT`, `RANDOMKEY`, `RENAMENX`, `SCAN`, `BITOP`,
* list: `BRPOPLPUSH`, `RPOPLPUSH`,
* set: `SINTERSTORE`, `SDIFFSTORE`, `SINTER`, `SMOVE`, `SUNIONSTORE`,
* sorted set: `ZINTERSTORE`, `ZUNIONSTORE`,
* pub/sub: `PUBSUB`, `PUNSUBSCRIBE`, `UNSUBSCRIBE`,

others: `PFADD`, `PFCOUNT`, `PFMERGE`,
`EVAL`, `EVALSHA`, `SCRIPT`,
`WATCH`, `UNWATCH`, `EXEC`, `DISCARD`, `MULTI`,
`SELECT`, `QUIT`, `ECHO`, `AUTH`,
`CLUSTER`, `BGREWRITEAOF`, `BGSAVE`, `CLIENT`, `COMMAND`, `CONFIG`,
`DBSIZE`, `DEBUG`, `FLUSHALL`, `FLUSHDB`, `LASTSAVE`, `MONITOR`,
`ROLE`, `SAVE`, `SHUTDOWN`, `SLAVEOF`, `SLOWLOG`, `SYNC`, `TIME`,

For more information please read [here (CN)](https://github.com/HunanTV/redis-cerberus/wiki/Redis-%E9%9B%86%E7%BE%A4%E4%BB%A3%E7%90%86%E5%9F%BA%E6%9C%AC%E5%8E%9F%E7%90%86%E4%B8%8E%E4%BD%BF%E7%94%A8).
