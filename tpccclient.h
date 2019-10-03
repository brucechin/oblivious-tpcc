#pragma once

#include <stdint.h>
#include "clock.h"
#include "randomgenerator.h"
#include "tpccdb.h"
namespace tpcc {
class RandomGenerator;
}



class TPCCClient{
public:
    TPCCClient(Clock* clock, tpcc::RandomGenerator* generator, TPCCDB* db);
    ~TPCCClient();

    // void doStockLevel();
    // void doOrderStatus();
    void doDelivery();
    void doPayment();
    bool doNewOrder();

    void doOne();

    TPCCDB* db() { return db_; }

    
    Clock* clock_;
    tpcc::RandomGenerator* generator_;
    TPCCDB* db_;

    int num_warehouses_ = 2;
    int remote_item_milli_p_ = 600;
    static const int MAX_OL_QUANTITY = 10;
    

};
