#ifndef PREFETCHER_TAGGED
#define PREFETCHER_TAGGED

#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <vector>
#include "pin.H"

#include "prefetcher_base.hh"

using namespace std;



class TaggedPrefetcher:BasePrefetcher{
	public:
		void train_prefetcher(ADDRINT addr, ADDRINT PC, int type) override;
        vector<ADDRINT> get_candidates(ADDRINT addr, ADDRINT PC, int type) override;
        string dump_stats() override;
};



#endif