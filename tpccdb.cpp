#include "tpccdb.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>
#include "assert.h"

using std::vector;

TPCCUndo::~TPCCUndo() {
    STLDeleteValues(&modified_warehouses_);
    STLDeleteValues(&modified_districts_);
    STLDeleteValues(&modified_customers_);
    STLDeleteValues(&modified_stock_);
    STLDeleteValues(&modified_orders_);
    STLDeleteValues(&modified_order_lines_);
    STLDeleteElements(&deleted_new_orders_);
}

template <typename T>
static void copyIfNeeded(typename std::unordered_map<T*, T*>* map, T* source) {
    typedef typename std::unordered_map<T*, T*> MapType;
    std::pair<typename MapType::iterator, bool> result = map->insert(
            typename MapType::value_type(source, NULL));
    if (result.second) {
        // we did the insert: copy the value
        assert(result.first->second == NULL);
        result.first->second = new T(*source);
    } else {
        assert(result.first->second != NULL);
    }    
}

void TPCCUndo::save(Warehouse* w) {
    copyIfNeeded(&modified_warehouses_, w);
}
void TPCCUndo::save(District* d) {
    copyIfNeeded(&modified_districts_, d);
}
void TPCCUndo::save(Customer* c) {
    copyIfNeeded(&modified_customers_, c);
}
void TPCCUndo::save(Stock* s) {
    copyIfNeeded(&modified_stock_, s);
}
void TPCCUndo::save(Order* o) {
    copyIfNeeded(&modified_orders_, o);
}
void TPCCUndo::save(OrderLine* ol) {
    copyIfNeeded(&modified_order_lines_, ol);
}

void TPCCUndo::inserted(const Order* o) {
    assert(inserted_orders_.find(o) == inserted_orders_.end());
    inserted_orders_.insert(o);
}
void TPCCUndo::inserted(const OrderLine* ol) {
    assert(inserted_order_lines_.find(ol) == inserted_order_lines_.end());
    inserted_order_lines_.insert(ol);
}
void TPCCUndo::inserted(const NewOrder* no) {
    assert(inserted_new_orders_.find(no) == inserted_new_orders_.end());
    inserted_new_orders_.insert(no);
}
void TPCCUndo::inserted(const History* h) {
    assert(inserted_history_.find(h) == inserted_history_.end());
    inserted_history_.insert(h);
}
void TPCCUndo::deleted(NewOrder* no) {
    assert(deleted_new_orders_.find(no) == deleted_new_orders_.end());
    deleted_new_orders_.insert(no);
}

void TPCCUndo::applied() {
    deleted_new_orders_.clear();
}



TPCCDB::~TPCCDB() {
    // Clean up the b-trees with this gross hack
    STLDeleteValues(&warehouses_);
    STLDeleteValues(&stock_);
    STLDeleteValues(&districts_);
    STLDeleteValues(&orders_);
    STLDeleteValues(&orderlines_);
    STLDeleteValues(&neworders_);
    STLDeleteValues(&history_);
    STLDeleteValues(&items_);
    STLDeleteValues(&customers_);
}


TPCCDB::TPCCDB(){
    
}

TPCCDB::WarehouseSet TPCCDB::newOrderRemoteWarehouses(Integer home_warehouse,
        const std::vector<NewOrderItem>& items) {
    WarehouseSet out;
    for (size_t i = 0; i < items.size(); ++i) {
        Integer supplyID = items[i].ol_supply_w_id;
        if (supplyID!= home_warehouse) {
            out.insert(supplyID);
        }
    }
    return out;
}


bool TPCCDB::findAndValidateItems(const vector<NewOrderItem>& items,
        vector<Item*>* item_tuples) {
    // CHEAT: Validate all items to see if we will need to abort
    item_tuples->resize(items.size());
    for (int i = 0; i < items.size(); ++i) {
        (*item_tuples)[i] = findItem(items[i].i_id);
        if ((*item_tuples)[i] == NULL) {
            return false;
        }
    }
    return true;
}

bool TPCCDB::newOrder(Integer warehouse_id, Integer district_id, Integer customer_id,
        const std::vector<NewOrderItem>& items, TPCCUndo** undo) {
    // perform the home part
    bool result = newOrderHome(warehouse_id, district_id, customer_id, items,  undo);
    if (!result) {
        return false;
    }

    // Process all remote warehouses
    WarehouseSet warehouses = newOrderRemoteWarehouses(warehouse_id, items);
    for (WarehouseSet::const_iterator i = warehouses.begin(); i != warehouses.end(); ++i) {
        result = newOrderRemote(warehouse_id, *i, items, undo);
        assert(result);
    }

    return true;
}


bool TPCCDB::newOrderRemote(Integer home_warehouse, Integer remote_warehouse,
        const vector<NewOrderItem>& items, TPCCUndo** undo) {
    // Validate all the items: needed so that we don't need to undo in order to execute this
    vector<Item*> item_tuples;
    if (!findAndValidateItems(items, &item_tuples)) {
        return false;
    }

    // We will not abort: allocate an undo buffer
    allocateUndo(undo);

    for (int i = 0; i < items.size(); ++i) {
        // Skip items that don't belong to remote warehouse
        if(items[i].getSupplyWarehouseID() != remote_warehouse)
            continue;
        }

        // update stock
        Stock* stock = findStock(items[i].getSupplyWarehouseID(), items[i].getItemID());
        if (undo != NULL) {
            (*undo)->save(stock);
        }

        Integer stock_quantity = stock->getStockQuantity();
        Integer item_ol_quantity = items[i].getQuantity(); 

        if((stock_quantity >= item_ol_quantity + Integer(INT_LENGTH, 10, party_)).reveal<bool>(PUBLIC)){
            stock->putStockQuantity(stock_quantity - item_ol_quantity);
        }else{
            stock->putStockQuantity(stock_quantity - item_ol_quantity + Integer(INT_LENGTH, 91, party_));
        }

        // if (stock->s_quantity >= items[i].ol_quantity + 10) {
        //     stock->s_quantity -= items[i].ol_quantity;
        // } else {
        //     stock->s_quantity = stock->s_quantity - items[i].ol_quantity + 91;
        // }
        stock->putStockYTD(stock->getStockYTD() + items[i].getQuantity());
        stock->putStockOrderCnt(stock->getStockOrderCnt() + Integer(INT_LENGTH, 1, party_));
        // newOrderHome calls newOrderRemote, so this is needed
        if (items[i].getSupplyWarehouseID != home_warehouse) {
            // remote order
            stock->putStockRemoteCnt(stock->getStockRemoteCnt + Integer(INT_LENGTH, 1, party_));
        }
    }

    return true;
}



bool TPCCTables::newOrderHome(Integer warehouse_id, Integer district_id, Integer customer_id,
        const vector<NewOrderItem>& items, TPCCUndo** undo) {

    // read those values first
    District* d = findDistrict(warehouse_id, district_id);
    Customer* c = findCustomer(warehouse_id, district_id, customer_id);

    // CHEAT: Validate all items to see if we will need to abort
    vector<Item*> item_tuples(items.size());
    if (!findAndValidateItems(items, &item_tuples)) {
        return false;
    }


    // We will not abort: update the status and the database state, allocate an undo buffer
    allocateUndo(undo);

    // Modify the order id to assign it
    if (undo != NULL) {
        (*undo)->save(d);
    }
    d->d_next_o_id += 1;

    Warehouse* w = findWarehouse(warehouse_id);

    Order* order = new Order(party_);
    order->o_w_id = warehouse_id;
    order->o_d_id = district_id;
    //TODO : there is problems with this initilaization because we need two parties to create secret share.
    order->o_id = Integer(INT_LENGTH, d->d_next_o_id, party_);
    order->o_c_id = customer_id;
    order->o_ol_cnt = Integer(INT_LENGTH, items.size(), party_);
    orders_.push_back(order);

    NewOrder* neworder = new NewOrder(party_);
    neworder->no_w_id = warehouse_id;
    neworder->no_d_id = district_id;
    neworder->no_o_id = order->o_id;
    neworders_.push_back(neworder);
    // Order* o = insertOrder(order);
    // NewOrder* no = insertNewOrder(warehouse_id, district_id, d->d_next_o_id);
    // if (undo != NULL) {
    //     (*undo)->inserted(o);
    //     (*undo)->inserted(no);
    // }

    OrderLine* line = new OrderLine(party_);
    line.ol_o_id = order->o_id;
    line.ol_d_id = district_id;
    line.ol_w_id = warehouse_id;

    for (int i = 0; i < items.size(); ++i) {
        line.ol_i_id = items[i].i_id;
        line.ol_supply_w_id = items[i].ol_supply_w_id;
        line.ol_quantity = items[i].ol_quantity;

        // Vertical Partitioning HACK: We read s_dist_xx from our local replica, assuming that
        // these columns are replicated everywhere.
        // TODO: I think this is unrealistic, since it will occupy ~23 MB per warehouse on all
        // replicas. Try the "two round" version in the future.
        Stock* stock = findStock(items[i].ol_supply_w_id, items[i].i_id);
        line.ol_amount = items[i].ol_quantity * item_tuples[i]->i_price;;
        orderlines_.push_back(line);
        //OrderLine* ol = insertOrderLine(line);
        // if (undo != NULL) {
        //     (*undo)->inserted(ol);
        // }
    }

    // Perform the "remote" part for this warehouse
    // TODO: It might be more efficient to merge this into the loop above, but this is simpler.
    bool result = newOrderRemote(warehouse_id, warehouse_id, items,  undo);
    ASSERT(result);

    return true;
}



void TPCCTables::payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id, Integer h_amount, const char* now,
        PaymentOutput* output, TPCCUndo** undo) {
    //~ printf("payment %d %d %d %d %d %f %s\n", warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount, now);
    Customer* customer = findCustomer(c_warehouse_id, c_district_id, customer_id);
    paymentHome(warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount,
            now, output, undo);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, output, undo);
}

void TPCCTables::payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, const char* c_last, Integer h_amount, TPCCUndo** undo) {
    //~ printf("payment %d %d %d %d %s %f %s\n", warehouse_id, district_id, c_warehouse_id, c_district_id, c_last, h_amount, now);
    Customer* customer = findCustomerByName(c_warehouse_id, c_district_id, c_last);
    paymentHome(warehouse_id, district_id, c_warehouse_id, c_district_id, customer->c_id, h_amount,
            now, output, undo);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, output, undo);
}



void TPCCTables::paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo) {
    Customer* customer = findCustomer(c_warehouse_id, c_district_id, c_id);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount,  undo);
}
void TPCCTables::paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, const char* c_last, Integer h_amount, TPCCUndo** undo) {
    Customer* customer = findCustomerByName(c_warehouse_id, c_district_id, c_last);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
}


void TPCCTables::paymentHome(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id, Integer h_amount, TPCCUndo** undo) {

    Warehouse* w = findWarehouse(warehouse_id);
    // if (undo != NULL) {
    //     allocateUndo(undo);
    //     (*undo)->save(w);
    // }
    w->w_ytd += h_amount;

    District* d = findDistrict(warehouse_id, district_id);
    // if (undo != NULL) {
    //     (*undo)->save(d);
    // }
    d->d_ytd += h_amount;

    // Insert the line into the history table
    History h(party_);
    h.h_w_id = warehouse_id;
    h.h_d_id = district_id;
    h.h_c_w_id = c_warehouse_id;
    h.h_c_d_id = c_district_id;
    h.h_c_id = customer_id;
    h.h_amount = h_amount;

    history_.push_back(h)
    // History* history = insertHistory(h);
    // if (undo != NULL) {
    //     (*undo)->inserted(history);
    // }

    // Zero all the customer fields: avoid uninitialized data for serialization
}



void TPCCTables::internalPaymentRemote(Integer warehouse_id, Integer district_id, Customer* c,
        Integer h_amount,  TPCCUndo** undo) {
    // if (undo != NULL) {
    //     allocateUndo(undo);
    //     (*undo)->save(c);
    // }
    c->c_balance -= h_amount;
    c->c_ytd_payment += h_amount;
    c->c_payment_cnt += 1;

}







// Allocates an undo buffer if needed, storing the pointer in *undo.
void allocateUndo(TPCCUndo** undo) {
    if (undo != NULL && *undo == NULL) {
        *undo = new TPCCUndo();
    }
}


Stock* TPCCDB::findStock(Integer w_id, Integer s_id) {
     
    int len = stock_.size();
    Integer wid_read = Integer(INT_LENGTH, 0, party_);
    Integer sid_read = Integer(INT_LENGTH, 0, party_);
    for(int i = 0; i < len; i++){
        readInteger(*stock_[i], sid_read, 0, 0, INT_LENGTH);
        readInteger(*stock_[i], wid_read, INT_LENGTH, 0, INT_LENGTH);
        bool res1 = (sid_read == s_id).reveal(PUBLIC);
        bool res2 = (wid_read == w_id).reveal(PUBLIC);
        if(res1 && res2){
            return stock_[i];
        }
    }
    return NULL;
}

Warehouse* TPCCDB::findWarehouse(Integer w_id){
    int len = warehouses_.size();
    Integer wid_read = Integer(INT_LENGTH, 0, party_);
    for(int i = 0; i < len; i++){
        readInteger(*warehouses_[i], wid_read, 0, 0, INT_LENGTH);
        if((w_id == wid_read).reveal(PUBLIC)){
            return warehouse_[i];
        }
    }
    return NULL;
}

Customer* TPCCDB::findCustomer(Integer w_id, Integer d_id, Integer c_id){
    inr len = customers_.size();
    Integer wid_read = Integer(INT_LENGTH, 0, party_);
    Integer did_read = Integer(INT_LENGTH, 0, party_);
    Integer cid_read = Integer(INT_LENGTH, 0, party_);
    for(int i = 0; i < len; i++){
        readInteger(*customers_[i], cid_read, 0, 0, INT_LENGTH);
        readInteger(*customers_[i], did_read, INT_LENGTH, 0, INT_LENGTH);
        readInteger(*customers_[i], wid_read, INT_LENGTH * 2, 0, INT_LENGTH);
        bool res1 = (w_id == wid_read).reveal(PUBLIC);
        bool res2 = (d_id == did_read).reveal(PUBLIC);
        bool res3 = (c_id == cid_read).reveal(PUBLIC);
        if(res1 && res2 && res3){
            return customers_[i];
        }
    }
    return NULL;
}