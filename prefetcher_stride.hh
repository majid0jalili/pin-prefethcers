#ifndef PREFETCHER_STRIDE
#define PREFETCHER_STRIDE

#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <vector>
#include "pin.H"

#include "prefetcher_base.hh"

#define IP_TRACKER_COUNT 1024
#define PREFETCH_DEGREE 3
#define LOG2_BLOCK_SIZE 6
#define LOG2_PAGE_SIZE 12

using namespace std;


class IP_TRACKER {
  public:
    // the IP we're tracking
    uint64_t ip;

    // the last address accessed by this IP
    uint64_t last_cl_addr;

    // the stride between the last two addresses accessed by this IP
    int64_t last_stride;

    // use LRU to evict old IP trackers
    uint32_t lru;

    IP_TRACKER () {
        ip = 0;
        last_cl_addr = 0;
        last_stride = 0;
        lru = 0;
    };
};


class StridePrefetcher:BasePrefetcher{
	public:
		void train_prefetcher(ADDRINT addr, ADDRINT PC, int type) override;
        vector<ADDRINT> get_candidates(ADDRINT addr, ADDRINT PC, int type) override;
		void l2c_prefetcher_initialize();
		StridePrefetcher();
		string dump_stats() override;
		
		
		IP_TRACKER trackers[IP_TRACKER_COUNT];
		ADDRINT hits;
		ADDRINT misses;
		
};





#endif