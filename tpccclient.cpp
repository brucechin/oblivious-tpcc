#include "tpccclient.h"
#include <cstdio>
#include <vector>


TPCCClient::~TPCCClient() {
    delete clock_;
    delete generator_;
    delete db_;
}

TPCCClient::TPCCClient(Clock* clock, tpcc::RandomGenerator* generator, TPCCDB* db) :
        clock_(clock),
        generator_(generator),
        db_(db) {
}


void TPCCClient::doDelivery() {

    vector<DeliveryOrderInfo> orders;
    Integer w_id = Integer(INT_LENGTH, generator_->number(1, Warehouse::MAX_WAREHOUSE_NUM), PUBLIC);
    db_->delivery(w_id, &orders, NULL);
    // if (orders.size() != District::NUM_PER_WAREHOUSE) {
    //     printf("Only delivered from %zd districts\n", orders.size());
    // }
}

void TPCCClient::doPayment() {
    int x = generator_->number(1, 100);
    
    Integer w_id = Integer(INT_LENGTH, generator_->number(1, Warehouse::MAX_WAREHOUSE_NUM), PUBLIC);
    Integer d_id = Integer(INT_LENGTH, generator_->number(1, District::NUM_PER_WAREHOUSE), PUBLIC);

    Integer c_w_id;
    Integer c_d_id;
    if (num_warehouses_ == 1 || x <= 85) {
        // 85%: paying through own warehouse (or there is only 1 warehouse)
        c_w_id = w_id;
        c_d_id = d_id;
    } else {
        // 15%: paying through another warehouse:
        // TODO : select in range [1, num_warehouses] excluding w_id
        c_w_id = Integer(INT_LENGTH, generator_->number(1, Warehouse::MAX_WAREHOUSE_NUM), PUBLIC);
        c_d_id = Integer(INT_LENGTH, generator_->number(1, 10), PUBLIC);
    }
    Integer h_amount  = Integer(INT_LENGTH, generator_->number(1, 10), PUBLIC);
    Integer c_id = Integer(INT_LENGTH, generator_->number(1, Customer::MAX_CUSTOMER_NUM), PUBLIC);
    db_->payment(w_id, d_id, c_w_id, c_d_id, c_id, h_amount, NULL);

}

bool TPCCClient::doNewOrder() {
    Integer w_id = Integer(INT_LENGTH, generator_->number(1, Warehouse::MAX_WAREHOUSE_NUM), PUBLIC);
    int ol_cnt = generator_->number(Order::MIN_OL_CNT, Order::MAX_OL_CNT);

    // 1% of transactions roll back
    //bool rollback = generator_->number(1, 100) == 1;

    vector<NewOrderItem> items(ol_cnt);
    for (int i = 0; i < ol_cnt; ++i) {
        if (i+1 == ol_cnt) {
            items[i].ol_i_id = Integer(INT_LENGTH, Item::NUM_ITEMS + 1, PUBLIC) ;
        } else {
            items[i].ol_i_id = Integer(INT_LENGTH, generator_->number(1, Item::NUM_ITEMS), PUBLIC);
        }

        // TPC-C suggests generating a number in range (1, 100) and selecting remote on 1
        // This provides more variation, and lets us tune the fraction of "remote" transactions.
        bool remote = generator_->number(1, 1000) <= remote_item_milli_p_;
        if (num_warehouses_ > 1 && remote) {
            //TODO : here we should assign a value different from w_id to ol_supply_w_id
            items[i].ol_supply_w_id = Integer(INT_LENGTH, generator_->number(1, Warehouse::MAX_WAREHOUSE_NUM), PUBLIC);
        } else {
            items[i].ol_supply_w_id = w_id;
        }
        items[i].ol_quantity = Integer(INT_LENGTH, generator_->number(1, MAX_OL_QUANTITY), PUBLIC);
    }
    Integer d_id = Integer(INT_LENGTH, generator_->number(1, District::NUM_PER_WAREHOUSE), PUBLIC);
    Integer c_id = Integer(INT_LENGTH, generator_->number(1, Customer::MAX_CUSTOMER_NUM), PUBLIC);
    bool result = db_->newOrder(w_id, d_id, c_id, items,  NULL);
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