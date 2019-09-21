#ifndef TPCCCLIENT_H__
#define TPCCCLIENT_H__

#include <stdint.h>

namespace tpcc {
class RandomGenerator;
}



class TPCCClient{
public:
    TPCCClient(Clock* clock, tpcc::RandomGenerator* generator, TPCCDB* db, int num_items,
            int num_warehouses, int districts_per_warehouse, int customers_per_district);
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

}