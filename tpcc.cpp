
#define __STDC_FORMAT_MACROS
#include <climits>
#include <cstdio>
#include <inttypes.h>
#include "clock.h"
#include "randomgenerator.h"
#include "tpccclient.h"
#include "tpccdb.h"

using namespace emp;
using namespace std;
//static const int NUM_TRANSACTIONS = 200000;

int main(int argc, char** argv) {
    
    int party, port;
    parse_party_and_port(argv, 2, &party, &port);
    TPCCDB* tables = new TPCCDB(party, port);
    tables->loadItems("./data/itemAlice.csv","./data/itemBob.csv");
    // SystemClock* clock = new SystemClock();
    
    //TODO : load csv file with encryption into tables

    // tpcc::RealRandomGenerator* random = new tpcc::RealRandomGenerator();
    // tpcc::NURandC cLoad = tpcc::NURandC::makeRandom(random);
    // random->setC(cLoad);


    // Client owns all the parameters
    // TPCCClient client(clock, random, tables, Item::NUM_ITEMS, static_cast<int>(num_warehouses),
    //         District::NUM_PER_WAREHOUSE, Customer::NUM_PER_DISTRICT);
    
    printf("Running... ");
    fflush(stdout);
    //int64_t begin = clock->getMicroseconds();
    // for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
    //     client.doOne();
    // }
    //int64_t end = clock->getMicroseconds();
    //int64_t microseconds = end - begin;
    //printf("%d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
    //        (microseconds + 500)/1000, NUM_TRANSACTIONS / (double) microseconds * 1000000.0);

    return 0;
}