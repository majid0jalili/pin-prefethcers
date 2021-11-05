#include "prefetcher_tagged.hh"
#include <vector>

void 
TaggedPrefetcher::train_prefetcher(ADDRINT addr, ADDRINT PC, int type)
{

}


vector<ADDRINT> 
TaggedPrefetcher::get_candidates(ADDRINT addr, ADDRINT PC, int type)
{
	vector<ADDRINT> candids;
	candids.push_back( (addr>>6)+1);
	candids.push_back( (addr>>6)+2);
	return candids;
}


string 
TaggedPrefetcher::dump_stats() 
{
	return "Tagged prefethcer has not spsecific stats";
}