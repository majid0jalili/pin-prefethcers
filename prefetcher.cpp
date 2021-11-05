/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable cache simulator
 *  data cache hierarchies
 */

#include "pin.H"

#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <vector>

#include "cache.H"
#include "pin_profile.H"


#include "prefetcher_tagged.hh"
#include "prefetcher_stride.hh"

using std::cerr;
using std::endl;
using namespace std;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "dcache.out", "specify dcache file name");
KNOB< BOOL > KnobTrackLoads(KNOB_MODE_WRITEONCE, "pintool", "tl", "0", "track individual loads -- increases profiling time");
KNOB< BOOL > KnobTrackStores(KNOB_MODE_WRITEONCE, "pintool", "ts", "0", "track individual stores -- increases profiling time");
KNOB< UINT32 > KnobThresholdHit(KNOB_MODE_WRITEONCE, "pintool", "rh", "100", "only report memops with hit count above threshold");
KNOB< UINT32 > KnobThresholdMiss(KNOB_MODE_WRITEONCE, "pintool", "rm", "100",
                                 "only report memops with miss count above threshold");
KNOB< UINT32 > KnobCacheSize(KNOB_MODE_WRITEONCE, "pintool", "c", "32", "cache size in kilobytes");
KNOB< UINT32 > KnobLineSize(KNOB_MODE_WRITEONCE, "pintool", "b", "32", "cache block size in bytes");
KNOB< UINT32 > KnobAssociativity(KNOB_MODE_WRITEONCE, "pintool", "a", "4", "cache associativity (1 for direct mapped)");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool represents a cache simulator.\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

map<ADDRINT, UINT32> addrMap;
vector<int> scope;

TaggedPrefetcher tagged;
StridePrefetcher* stride;

/* ===================================================================== */



VOID Access(ADDRINT addr, ADDRINT PC, int type)
{
	ADDRINT block_address = addr>>6;	
	// cout<<"original "<<hex<<addr<<" addr "<<hex<<block_address<<" next addr "<<block_address+1<<" pri addr "<<block_address-1<<endl;
	auto it = addrMap.find(block_address);
	if (it != addrMap.end()){
		scope[0]++;
	}else{
		scope[1]++;
		vector<ADDRINT> candids;
		candids = stride->get_candidates(addr, PC, type);
		scope[2] += candids.size();
		for (unsigned  int i = 0; i < candids.size(); i++){
			addrMap.insert({candids[i], 1});
		}
	}
		
}


/* ===================================================================== */
VOID LoadMulti(ADDRINT addr, UINT32 size, ADDRINT instId)
{
	Access(addr, instId, 0);
}
VOID StoreMulti(ADDRINT addr, UINT32 size, ADDRINT instId)
{
	Access(addr, instId, 1);
}

VOID LoadSingle(ADDRINT addr, ADDRINT instId)
{
	Access(addr, instId, 0);
}
VOID StoreSingle(ADDRINT addr, ADDRINT instId)
{
	Access(addr, instId, 1);
}




VOID Instruction(INS ins, void* v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Instrument each memory operand. If the operand is both read and written
    // it will be processed twice.
    // Iterating over memory operands ensures that instructions on IA-32 with
    // two read operands (such as SCAS and CMPS) are correctly handled.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        const BOOL single = (size <= 4);

        if (INS_MemoryOperandIsRead(ins, memOp))
        {
			// map sparse INS addresses to dense IDs
			const ADDRINT iaddr = INS_Address(ins);


			if (single)
			{
				INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadSingle, IARG_MEMORYOP_EA, memOp,  IARG_ADDRINT,
										 iaddr, IARG_END);
			}
			else
			{
				INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadMulti, IARG_MEMORYOP_EA, memOp, IARG_UINT32, size,
										  IARG_ADDRINT, iaddr, IARG_END);
			}
         
            
        }

        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
          
			const ADDRINT iaddr = INS_Address(ins);


			if (single)
			{
				INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)StoreSingle, IARG_MEMORYOP_EA, memOp,  IARG_ADDRINT,
										 iaddr, IARG_END);
			}
			else
			{
				INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)StoreMulti, IARG_MEMORYOP_EA, memOp, IARG_UINT32, size,
										  IARG_ADDRINT, iaddr, IARG_END);
			}
           
           
        }
    }
}

/* ===================================================================== */

VOID Fini(int code, VOID* v)
{
	
    std::ofstream out(KnobOutputFile.Value().c_str());

    // print D-cache profile
    // @todo what does this print

    
	out << "Covered "<<scope[0]<<endl;
	out << "Not Covered "<<scope[1]<<endl;
	out << "Scope "<<(scope[0]*1.0)/(scope[1]+scope[0])<<endl;
	out << "Prefetched "<<(scope[2])<<endl;
	out << "Overhead "<<(scope[2]*1.0)/(scope[1]+scope[0]+scope[0])<<endl;
	out << "Prefetcher stats "<<stride->dump_stats()<<endl;

    out.close();
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
	scope.push_back(0);
	scope.push_back(0);
	scope.push_back(0);
	stride = new StridePrefetcher();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
