#ifndef PREFETCHER_BASE
#define PREFETCHER_BASE

#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <vector>
#include "pin.H"

#include "pin_profile.H"




using namespace std;



class BasePrefetcher{
	public:
		virtual void train_prefetcher(ADDRINT addr, ADDRINT PC, int type);
		virtual vector<ADDRINT> get_candidates(ADDRINT addr, ADDRINT PC, int type);
		virtual string dump_stats();
};

#endif