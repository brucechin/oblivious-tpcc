#pragma once

#include <stdint.h>
#include "clock.h"
#include "randomgenerator.h"
#include "tpccdb_tmp.h"
#include "ctpl.h"

namespace tpcc {
class RandomGenerator;
}



class TPCCClient{
public:
    int num_warehouses_ = 2;
    int remote_item_milli_p_ = 600;
    static const int MAX_OL_QUANTITY = 10;
    const int num_threads = 10;


    Clock* clock_;
    tpcc::RandomGenerator* generator_;
    TPCCDB* db_;

    TPCCClient(Clock* clock, tpcc::RandomGenerator* generator, TPCCDB* db);
    ~TPCCClient();

    // void doStockLevel();
    // void doOrderStatus();
    void doDelivery();
    void doPayment();
    bool doNewOrder();
    bool doNewOrderBatch();
    void doOne();
    TPCCDB* db() { return db_; }

    
    std::thread t1;
    std::thread t2;
    
    

};
