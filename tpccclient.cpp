#include "tpccclient.h"
#include <cstdio>
#include <vector>


#include "clock.h"
//#include "randomgenerator.h"
#include "tpccdb.h"

TPCCClient::~TPCCClient() {
    delete clock_;
    delete generator_;
    delete db_;
}

TPCCClient::TPCCClient(Clock* clock, tpcc::RandomGenerator* generator, TPCCDB* d) :
        clock_(clock),
        generator_(generator),
        db_(db) {
    ASSERT(clock_ != NULL);
    ASSERT(generator_ != NULL);
    ASSERT(db_ != NULL);
}


void TPCCClient::doDelivery() {
    int carrier = generator_->number(Order::MIN_CARRIER_ID, Order::MAX_CARRIER_ID);

    vector<DeliveryOrderInfo> orders;
    db_->delivery(generateWarehouse(), carrier,  &orders, NULL);
    // if (orders.size() != District::NUM_PER_WAREHOUSE) {
    //     printf("Only delivered from %zd districts\n", orders.size());
    // }
}

void TPCCClient::doPayment() {
    int x = generator_->number(1, 100);
    
    int32_t w_id = generateWarehouse();
    int32_t d_id = generateDistrict();

    int32_t c_w_id;
    int32_t c_d_id;
    if (num_warehouses_ == 1 || x <= 85) {
        // 85%: paying through own warehouse (or there is only 1 warehouse)
        c_w_id = w_id;
        c_d_id = d_id;
    } else {
        // 15%: paying through another warehouse:
        // select in range [1, num_warehouses] excluding w_id
        c_w_id = generator_->numberExcluding(1, num_warehouses_, w_id);
        ASSERT(c_w_id != w_id);
        c_d_id = generateDistrict();
    }
    float h_amount = generator_->fixedPoint(2, MIN_PAYMENT_AMOUNT, MAX_PAYMENT_AMOUNT);

    db_->payment(w_id, d_id, c_w_id, c_d_id, generateCID(), h_amount, NULL);

}

bool TPCCClient::doNewOrder() {
    int32_t w_id = generateWarehouse();
    int ol_cnt = generator_->number(Order::MIN_OL_CNT, Order::MAX_OL_CNT);

    // 1% of transactions roll back
    bool rollback = generator_->number(1, 100) == 1;

    vector<NewOrderItem> items(ol_cnt);
    for (int i = 0; i < ol_cnt; ++i) {
        if (rollback && i+1 == ol_cnt) {
            items[i].i_id = Item::NUM_ITEMS + 1;
        } else {
            items[i].i_id = generateItemID();
        }

        // TPC-C suggests generating a number in range (1, 100) and selecting remote on 1
        // This provides more variation, and lets us tune the fraction of "remote" transactions.
        bool remote = generator_->number(1, 1000) <= remote_item_milli_p_;
        if (num_warehouses_ > 1 && remote) {
            items[i].ol_supply_w_id = generator_->numberExcluding(1, num_warehouses_, w_id);
        } else {
            items[i].ol_supply_w_id = w_id;
        }
        items[i].ol_quantity = generator_->number(1, MAX_OL_QUANTITY);
    }

    bool result = db_->newOrder(
            w_id, generateDistrict(), generateCID(), items,  NULL);
    ASSERT(result == !rollback);
    return result;
}

void TPCCClient::doOne() {
    // This is not strictly accurate: The requirement is for certain *minimum* percentages to be
    // maintained. This is close to the right thing, but not precisely correct.
    // See TPC-C 5.2.4 (page 68).
    int x = generator_->number(1, 100);
    if (x <= 5) { 
        doDelivery();
    } else if (x <= 12+43) { 
        doPayment();
    } else {  
        doNewOrder();
    }
}