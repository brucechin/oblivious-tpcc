
#define __STDC_FORMAT_MACROS
#include <climits>
#include <cstdio>
#include <inttypes.h>
#include "clock.h"
#include "randomgenerator.h"
#include "tpccclient.h"
#include "tpccdb_tmp.h"
#include <unistd.h>
#include <cstring>
#include<sstream>
#include<string>
#include<iostream>
#include<fstream>
#include <thread>
#include <chrono>
#include <utility>
#include "ctpl.h"


using namespace emp;
using namespace std;

static const int NUM_TRANSACTIONS = 3;



int main(int argc, char** argv) {
    
    int party, port;
    int num_warehouses = 2;
    parse_party_and_port(argv, 2, &party, &port);
    
    NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);
	setup_semi_honest(io, party);
    TPCCDB* tables = new TPCCDB(party, port);

    tables->loadFromCSV("Alice.csv", "Bob.csv");
    io->flush();    

    SystemClock* clock = new SystemClock();
    

    tpcc::RealRandomGenerator* random = new tpcc::RealRandomGenerator();
    tpcc::NURandC cLoad = tpcc::NURandC::makeRandom(random);
    random->setC(cLoad);

    cout << "random generator init "<<endl;
    // Client owns all the parameters
    TPCCClient client(clock, random, tables);
    cout << "tpcc client init" <<endl;



    int64_t begin = clock->getMicroseconds();

    for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
        client.doNewOrder();
        io->flush();
    }
    
    

    int64_t end = clock->getMicroseconds();
    int64_t microseconds = end - begin;
    printf("not batching %d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
           (microseconds + 500)/1000, NUM_TRANSACTIONS / (double) microseconds * 1000000.0);


    //batching speedup testing

    begin = clock->getMicroseconds();

    for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
        client.doNewOrderBatch();
        io->flush();
    }
    
    delete io;

    end = clock->getMicroseconds();
    microseconds = end - begin;
    printf("batching %d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
           (microseconds + 500)/1000, NUM_TRANSACTIONS / (double) microseconds * 1000000.0);




    return 0;
}
