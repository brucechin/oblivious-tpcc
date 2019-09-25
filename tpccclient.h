#pragma once

#include <stdint.h>
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

private:
    
    Clock* clock_;
    tpcc::RandomGenerator* generator_;
    TPCCDB* db_;

};
