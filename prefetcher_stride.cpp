#include "prefetcher_stride.hh"
#include <vector>








StridePrefetcher::StridePrefetcher() 
{
	cout<<"StridePrefetcher::l2c_prefetcher_initializ"<<endl;
    for (int i=0; i<IP_TRACKER_COUNT; i++)
	{
		trackers[i].lru = i;
	}
    hits = 0;
    misses = 0;
	cout<<"StridePrefetcher::l2c_prefetcher_initializ Done "<<IP_TRACKER_COUNT<<endl;
}



void 
StridePrefetcher::train_prefetcher(ADDRINT addr, ADDRINT PC, int type)
{
	// cout<<"StridePrefetcher::train_prefetcher"<<endl;

}


vector<ADDRINT> 
StridePrefetcher::get_candidates(ADDRINT addr, ADDRINT PC, int type)
{
	// cout<<"StridePrefetcher::get_candidates"<<endl;
	vector<ADDRINT> candids;
	
	  // check for a tracker hit
    uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;

    int index = -1;
    for (index=0; index<IP_TRACKER_COUNT; index++) {
        if (trackers[index].ip == PC)
            break;
    }

    // this is a new IP that doesn't have a tracker yet, so allocate one
    if (index == IP_TRACKER_COUNT) {
		misses += 1;
        for (index=0; index<IP_TRACKER_COUNT; index++) {
            if (trackers[index].lru == (IP_TRACKER_COUNT-1))
                break;
        }

        trackers[index].ip = PC;
        trackers[index].last_cl_addr = cl_addr;
        trackers[index].last_stride = 0;

        //cout << "[IP_STRIDE] MISS index: " << index << " lru: " << trackers[index].lru << " ip: " << hex << ip << " cl_addr: " << cl_addr << dec << endl;

        for (int i=0; i<IP_TRACKER_COUNT; i++) {
            if (trackers[i].lru < trackers[index].lru)
                trackers[i].lru++;
        }
        trackers[index].lru = 0;

        return candids;
    }
	hits += 1;
    // sanity check
    // at this point we should know a matching tracker index
    if (index == -1)
        assert(0);

    // calculate the stride between the current address and the last address
    // this bit appears overly complicated because we're calculating
    // differences between unsigned address variables
    int64_t stride = 0;
    if (cl_addr > trackers[index].last_cl_addr)
        stride = cl_addr - trackers[index].last_cl_addr;
    else {
        stride = trackers[index].last_cl_addr - cl_addr;
        stride *= -1;
    }

    //cout << "[IP_STRIDE] HIT  index: " << index << " lru: " << trackers[index].lru << " ip: " << hex << ip << " cl_addr: " << cl_addr << dec << " stride: " << stride << endl;

    // don't do anything if we somehow saw the same address twice in a row
    if (stride == 0)
        return candids;

    // only do any prefetching if there's a pattern of seeing the same
    // stride more than once
    if (stride == trackers[index].last_stride) {
		// cout<<"stride "<<stride<<endl;
        // do some prefetching
        for (int i=0; i<PREFETCH_DEGREE; i++) {
            uint64_t pf_address = (cl_addr + (stride*(i+1)));

            // only issue a prefetch if the prefetch address is in the same 4 KB page 
            // as the current demand access address
            // if ((pf_address >> LOG2_PAGE_SIZE) != (addr >> LOG2_PAGE_SIZE))
                // break;

           
            candids.push_back(pf_address);
	      
        }
    }

    trackers[index].last_cl_addr = cl_addr;
    trackers[index].last_stride = stride;

    for (int i=0; i<IP_TRACKER_COUNT; i++) {
        if (trackers[i].lru < trackers[index].lru)
            trackers[i].lru++;
    }
    trackers[index].lru = 0;

	
	
	
	return candids;
}


string 
StridePrefetcher::dump_stats() 
{
	
	std::stringstream ss;
	ss <<"Stride hits: " <<hits<<" Stride  misses: "<<misses<<endl;
	cout<<ss.str()<<endl;
	return ss.str();
}
