
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

static const int NUM_TRANSACTIONS = 5;

void execute(int id){
    std::this_thread::sleep_for(std::chrono::milliseconds(id * 10));
    cout << id << " start new order" << endl;
}

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
    TPCCClient* client = new TPCCClient(clock, random, tables);
    cout << "tpcc client init" <<endl;

    vector<std::thread*> pool;
    int64_t begin = clock->getMicroseconds();


    for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
        cout << i << "th transaction start \n\n\n"<<endl;
        pool.push_back(new std::thread(&TPCCClient::doPayment, client));
        
    }

    for(int i = 0; i < NUM_TRANSACTIONS; ++i){
        pool[i]->join();
        cout << i << "th txn joins" << endl;
        
    }
    io->flush();
    delete io;
    //delete client;
    

    int64_t end = clock->getMicroseconds();
    int64_t microseconds = end - begin;
    printf("%d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
           (microseconds + 500)/1000, NUM_TRANSACTIONS / (double) microseconds * 1000000.0);

    return 0;
}
