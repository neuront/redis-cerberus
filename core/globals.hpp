#ifndef __CERBERUS_GLOBALS_HPP__
#define __CERBERUS_GLOBALS_HPP__

#include <set>
#include <vector>

#include "common.hpp"
#include "concurrence.hpp"
#include "utils/pointer.h"
#include "utils/address.hpp"
#include "utils/object_pool.hpp"

namespace cerb_global {

    extern std::vector<cerb::ListenThread> all_threads;
    extern thread_local cerb::msize_t allocated_buffer;
    extern thread_local util::ObjectPool<util::MemPage> page_pool;

    void set_remotes(std::set<util::Address> remotes);
    std::set<util::Address> get_remotes();

    void set_cluster_req_full_cov(bool c);
    bool cluster_req_full_cov();

}

#endif /* __CERBERUS_GLOBALS_HPP__ */
