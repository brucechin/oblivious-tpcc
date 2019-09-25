#include "tpccdb.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>


using namespace std;

// Allocates an undo buffer if needed, storing the pointer in *undo.
void allocateUndo(TPCCUndo** undo) {
    if (undo != NULL && *undo == NULL) {
        *undo = new TPCCUndo();
    }
}

TPCCUndo::~TPCCUndo() {
    // STLDeleteValues(&modified_warehouses_);
    // STLDeleteValues(&modified_districts_);
    // STLDeleteValues(&modified_customers_);
    // STLDeleteValues(&modified_stock_);
    // STLDeleteValues(&modified_orders_);
    // STLDeleteValues(&modified_order_lines_);
    // STLDeleteElements(&deleted_new_orders_);
}

template <typename T>
static void copyIfNeeded(typename std::unordered_map<T*, T*>* map, T* source) {
    typedef typename std::unordered_map<T*, T*> MapType;
    std::pair<typename MapType::iterator, bool> result = map->insert(
            typename MapType::value_type(source, NULL));
    if (result.second) {
        // we did the insert: copy the value
        //ASSERT(result.first->second == NULL);
        result.first->second = new T(*source);
    } else {
        //ASSERT(result.first->second != NULL);
    }    
}

// void TPCCUndo::save(Warehouse* w) {
//     copyIfNeeded(&modified_warehouses_, w);
// }
// void TPCCUndo::save(District* d) {
//     copyIfNeeded(&modified_districts_, d);
// }
// void TPCCUndo::save(Customer* c) {
//     copyIfNeeded(&modified_customers_, c);
// }
// void TPCCUndo::save(Stock* s) {
//     copyIfNeeded(&modified_stock_, s);
// }
// void TPCCUndo::save(Order* o) {
//     copyIfNeeded(&modified_orders_, o);
// }
// void TPCCUndo::save(OrderLine* ol) {
//     copyIfNeeded(&modified_order_lines_, ol);
// }

// void TPCCUndo::inserted(const Order* o) {
//     //ASSERT(inserted_orders_.find(o) == inserted_orders_.end());
//     inserted_orders_.insert(o);
// }
// void TPCCUndo::inserted(const OrderLine* ol) {
//     //ASSERT(inserted_order_lines_.find(ol) == inserted_order_lines_.end());
//     inserted_order_lines_.insert(ol);
// }
// void TPCCUndo::inserted(const NewOrder* no) {
//     //ASSERT(inserted_new_orders_.find(no) == inserted_new_orders_.end());
//     inserted_new_orders_.insert(no);
// }
// void TPCCUndo::inserted(const History* h) {
//     //ASSERT(inserted_history_.find(h) == inserted_history_.end());
//     inserted_history_.insert(h);
// }
// void TPCCUndo::deleted(NewOrder* no) {
//     //ASSERT(deleted_new_orders_.find(no) == deleted_new_orders_.end());
//     deleted_new_orders_.insert(no);
// }

// void TPCCUndo::applied() {
//     deleted_new_orders_.clear();
// }



TPCCDB::~TPCCDB() {
    // Clean up the b-trees with this gross hack
    // STLDeleteValues(&warehouses_);
    // STLDeleteValues(&stocks_);
    // STLDeleteValues(&districts_);
    // STLDeleteValues(&orders_);
    // STLDeleteValues(&orderlines_);
    // STLDeleteValues(&neworders_);
    // STLDeleteValues(&history_);
    // STLDeleteValues(&items_);
    // STLDeleteValues(&customers_);
}


TPCCDB::TPCCDB(int party, int port) :
    party_(party),
    port_(port) {
    
}

vector<Integer> TPCCDB::newOrderRemoteWarehouses(Integer home_warehouse,
        const std::vector<NewOrderItem>& items) {
    vector<Integer> out;
    for (size_t i = 0; i < items.size(); ++i) {
        Integer supplyID = items[i].ol_supply_w_id;
        if ((supplyID != home_warehouse).reveal<bool>(PUBLIC)) {
            out.push_back(supplyID);
        }
    }
    return out;
}


bool TPCCDB::findAndValidateItems(const vector<NewOrderItem>& items,
        vector<Item*>* item_tuples) {
    // CHEAT: Validate all items to see if we will need to abort
    item_tuples->resize(items.size());
    for (int i = 0; i < items.size(); ++i) {
        (*item_tuples)[i] = findItem(items[i].ol_i_id);
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
    vector<Integer> warehouses = newOrderRemoteWarehouses(warehouse_id, items);

    for(int i = 0; i < warehouses.size(); i++){
        result = newOrderRemote(warehouse_id, warehouses[i], items, undo);
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
    //allocateUndo(undo);

    for (int i = 0; i < items.size(); ++i) {
        // Skip items that don't belong to remote warehouse
        if((items[i].ol_supply_w_id != remote_warehouse).reveal<bool>(PUBLIC)){
            continue;
        }

        // update stock
        Stock* stock = findStock(items[i].ol_supply_w_id, items[i].ol_i_id);
        // if (undo != NULL) {
        //     (*undo)->save(stock);
        // }

        Integer stock_quantity = stock->s_quantity;
        Integer item_ol_quantity = items[i].ol_quantity; 

        if((stock_quantity >= item_ol_quantity + Integer(INT_LENGTH, 10, PUBLIC)).reveal<bool>(PUBLIC)){
            stock->s_quantity = (stock_quantity - item_ol_quantity);
        }else{
            stock->s_quantity = (stock_quantity - item_ol_quantity + Integer(INT_LENGTH, 91, PUBLIC));
        }

        // if (stock->s_quantity >= items[i].ol_quantity + 10) {
        //     stock->s_quantity -= items[i].ol_quantity;
        // } else {
        //     stock->s_quantity = stock->s_quantity - items[i].ol_quantity + 91;
        // }
        stock->s_ytd = (stock->s_ytd + items[i].ol_quantity);
        stock->s_order_cnt = (stock->s_order_cnt + Integer(INT_LENGTH, 1, PUBLIC));
        // newOrderHome calls newOrderRemote, so this is needed
        if ((items[i].ol_supply_w_id != home_warehouse).reveal<bool>(PUBLIC)) {
            // remote order
            stock->s_remote_cnt = (stock->s_remote_cnt + Integer(INT_LENGTH, 1, PUBLIC));
        }
    }

    return true;
}



bool TPCCDB::newOrderHome(Integer warehouse_id, Integer district_id, Integer customer_id,
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
    //allocateUndo(undo);

    // Modify the order id to assign it
    // if (undo != NULL) {
    //     (*undo)->save(d);
    // }
    d->d_next_o_id = d->d_next_o_id + Integer(INT_LENGTH, 1, PUBLIC);

    Warehouse* w = findWarehouse(warehouse_id);

    Order* order = new Order(party_);
    order->o_w_id = warehouse_id;
    order->o_d_id = district_id;
    //TODO : there is problems with this initilaization because we need two parties to create secret share.
    order->o_id = d->d_next_o_id;
    order->o_c_id = customer_id;
    order->o_ol_cnt = Integer(INT_LENGTH, items.size(), PUBLIC);
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
    line->ol_o_id = order->o_id;
    line->ol_d_id = district_id;
    line->ol_w_id = warehouse_id;

    for (int i = 0; i < items.size(); ++i) {
        line->ol_number = Integer(INT_LENGTH, i + 1, PUBLIC);
        line->ol_i_id = items[i].ol_i_id;
        line->ol_supply_w_id = items[i].ol_supply_w_id;
        line->ol_quantity = items[i].ol_quantity;

        // Vertical Partitioning HACK: We read s_dist_xx from our local replica, assuming that
        // these columns are replicated everywhere.
        // TODO: I think this is unrealistic, since it will occupy ~23 MB per warehouse on all
        // replicas. Try the "two round" version in the future.
        Stock* stock = findStock(items[i].ol_supply_w_id, items[i].ol_i_id);
        line->ol_amount = items[i].ol_quantity * item_tuples[i]->i_price;;
        orderlines_.push_back(line);
        //OrderLine* ol = insertOrderLine(line);
        // if (undo != NULL) {
        //     (*undo)->inserted(ol);
        // }
    }

    // Perform the "remote" part for this warehouse
    // TODO: It might be more efficient to merge this into the loop above, but this is simpler.
    bool result = newOrderRemote(warehouse_id, warehouse_id, items,  undo);
    //ASSERT(result);

    return true;
}



void TPCCDB::payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id, Integer h_amount,  TPCCUndo** undo) {
    //~ printf("payment %d %d %d %d %d %f %s\n", warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount, now);
    Customer* customer = findCustomer(c_warehouse_id, c_district_id, customer_id);
    paymentHome(warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount, undo);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
}



void TPCCDB::paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo) {
    Customer* customer = findCustomer(c_warehouse_id, c_district_id, c_id);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
}


void TPCCDB::paymentHome(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id, Integer h_amount, TPCCUndo** undo) {

    Warehouse* w = findWarehouse(warehouse_id);
    // if (undo != NULL) {
    //     //allocateUndo(undo);
    //     (*undo)->save(w);
    // }
    w->w_ytd = w->w_ytd + h_amount;

    District* d = findDistrict(warehouse_id, district_id);
    // if (undo != NULL) {
    //     (*undo)->save(d);
    // }
    d->d_ytd = d->d_ytd + h_amount;

    // Insert the line into the history table
    History* h = new History(party_);
    h->h_w_id = warehouse_id;
    h->h_d_id = district_id;
    h->h_c_w_id = c_warehouse_id;
    h->h_c_d_id = c_district_id;
    h->h_c_id = customer_id;
    h->h_amount = h_amount;

    history_.push_back(h);
    // History* history = insertHistory(h);
    // if (undo != NULL) {
    //     (*undo)->inserted(history);
    // }

    // Zero all the customer fields: avoid uninitialized data for serialization
}



void TPCCDB::internalPaymentRemote(Integer warehouse_id, Integer district_id, Customer* c,
        Integer h_amount,  TPCCUndo** undo) {
    // if (undo != NULL) {
    //     //allocateUndo(undo);
    //     (*undo)->save(c);
    // }
    c->c_balance = c->c_balance - h_amount;
    c->c_ytd_payment = c->c_ytd_payment + h_amount;
    c->c_payment_cnt = c->c_payment_cnt + Integer(INT_LENGTH, 1, PUBLIC);

}



void TPCCDB::delivery(Integer warehouse_id, Integer carrier_id, 
        std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo) {
    //~ printf("delivery %d %d %s\n", warehouse_id, carrier_id, now);
    //allocateUndo(undo);
    orders->clear();

    for (int32_t index = 1; index <= District::NUM_PER_WAREHOUSE; ++index) {
        // Find and remove the lowest numbered order for the district
        // int64_t key = makeNewOrderKey(warehouse_id, d_id, 1);
        // NewOrderMap::iterator iterator = neworders_.lower_bound(key);
        // NewOrder* neworder = NULL;
        // if (iterator != neworders_.end()) {
        //     neworder = iterator->second;
        //     //ASSERT(neworder != NULL);
        // }
        Integer d_id = Integer(INT_LENGTH, index, PUBLIC);

        NewOrder* neworder = findNewOrder(warehouse_id, d_id, Integer(INT_LENGTH, 1, PUBLIC));
        if (neworder == NULL || (neworder->no_d_id != d_id).reveal<bool>(PUBLIC) || (neworder->no_w_id != warehouse_id).reveal<bool>(PUBLIC)) {
            // No orders for this district
            // TODO: 2.7.4.2: If this occurs in max(1%, 1) of transactions, report it (???)
            continue;
        }
        //ASSERT(neworder->no_d_id == d_id && neworder->no_w_id == warehouse_id);
        Integer o_id = neworder->no_o_id;
        //neworders_.erase(iterator);
        // if (undo != NULL) {
        //     (*undo)->deleted(neworder);
        // } else {
        //     delete neworder;
        // }

        DeliveryOrderInfo order;
        order.d_id = d_id;
        order.o_id = o_id;
        orders->push_back(order);

        Order* o = findOrder(warehouse_id, d_id, o_id);
        //ASSERT(o->o_carrier_id == Order::NULL_CARRIER_ID);
        // if (undo != NULL) {
        //     (*undo)->save(o);
        // }

        //TODO: which party shall I use to encrypt "value"
        Integer total = Integer(INT_LENGTH, 0, PUBLIC);
        Integer i = Integer(INT_LENGTH, 1, PUBLIC);
        while((i <= o->o_ol_cnt).reveal<bool>(PUBLIC)){
            i = i + Integer(INT_LENGTH, 1, PUBLIC);
            OrderLine* line = findOrderLine(warehouse_id, d_id, o_id, i);
            // if (undo != NULL) {
            //     (*undo)->save(line);
            // }
            total = total + line->ol_amount;
        }

        Customer* c = findCustomer(warehouse_id, d_id, o->o_c_id);
        // if (undo != NULL) {
        //     (*undo)->save(c);
        // }
        c->c_balance = c->c_balance + total;
        c->c_delivery_cnt = c->c_delivery_cnt + Integer(INT_LENGTH, 1, PUBLIC);
    }
}






Stock* TPCCDB::findStock(Integer w_id, Integer i_id) {
     
    int len = stocks_.size();
    for(int i = 0; i < len; i++){
        bool res1 = (stocks_[i]->s_i_id == i_id).reveal(PUBLIC);
        bool res2 = (stocks_[i]->s_w_id == w_id).reveal(PUBLIC);
        if(res1 && res2){
            return stocks_[i];
        }
    }
    return NULL;
}

Warehouse* TPCCDB::findWarehouse(Integer w_id){
    int len = warehouses_.size();
    for(int i = 0; i < len; i++){
        if((w_id == warehouses_[i]->w_id).reveal(PUBLIC)){
            return warehouses_[i];
        }
    }
    return NULL;
}

Customer* TPCCDB::findCustomer(Integer w_id, Integer d_id, Integer c_id){
    int len = customers_.size();
    for(int i = 0; i < len; i++){
        bool res1 = (w_id == customers_[i]->c_w_id).reveal(PUBLIC);
        bool res2 = (d_id == customers_[i]->c_d_id).reveal(PUBLIC);
        bool res3 = (c_id == customers_[i]->c_id).reveal(PUBLIC);
        if(res1 && res2 && res3){
            return customers_[i];
        }
    }
    return NULL;
}

NewOrder* TPCCDB::findNewOrder(Integer w_id, Integer d_id, Integer o_id){
    int len = neworders_.size();
    for(int i = 0; i < len; i++){
        bool res1 = (w_id == neworders_[i]->no_w_id).reveal(PUBLIC);
        bool res2 = (d_id == neworders_[i]->no_d_id).reveal(PUBLIC);
        bool res3 = (o_id == neworders_[i]->no_o_id).reveal(PUBLIC);
        if(res1 && res2 && res3){
            return neworders_[i];
        }
    }
    return NULL;
}

Item* TPCCDB::findItem(Integer id) {
    int len = items_.size();
    for(int i = 0; i < len; i++){
        if((id == items_[i]->i_id).reveal<bool>(PUBLIC)){
            return items_[i];
        }
    }
    return NULL;
}


OrderLine* TPCCDB::findOrderLine(Integer w_id, Integer d_id, Integer o_id, Integer number){
    int len = orderlines_.size();
    for(int i = 0; i < len; i++){
        bool res1 = (w_id == orderlines_[i]->ol_w_id).reveal(PUBLIC);
        bool res2 = (d_id == orderlines_[i]->ol_d_id).reveal(PUBLIC);
        bool res3 = (o_id == orderlines_[i]->ol_o_id).reveal(PUBLIC);
        bool res4 = (number == orderlines_[i]->ol_number).reveal(PUBLIC);

        if(res1 && res2 && res3 && res4){
            //TODO : this is not oblivious 
            return orderlines_[i];
        }
    }
    return NULL;
}


int numOfRows(string filename){
    //count lines of the csv file
    int res = 0;
    std::ifstream f(filename);
    std::string line;
    while(getline(f, line, '\n')){
        res++;
    }
    f.close();
    return res;

}

void TPCCDB::loadItems(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer i_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer i_price = Integer(INT_LENGTH, int(std::stof(row[2]) * 100), party_);// price is stored in cents
            Integer i_im_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            items_.push_back(new Item(i_id, i_im_id, i_price, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer i_id = Integer(INT_LENGTH, 0, party_);
            Integer i_price = Integer(INT_LENGTH, 0, party_);
            Integer i_im_id = Integer(INT_LENGTH, 0, party_);
            items_.push_back(new Item(i_id, i_im_id, i_price, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer i_id = Integer(INT_LENGTH, 0, party_);
            Integer i_price = Integer(INT_LENGTH, 0, party_);
            Integer i_im_id = Integer(INT_LENGTH, 0, party_);
            items_.push_back(new Item(i_id, i_im_id, i_price, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer i_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer i_price = Integer(INT_LENGTH, int(std::stof(row[2]) * 100), party_);// price is stored in cents
            Integer i_im_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            items_.push_back(new Item(i_id, i_im_id, i_price, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Item table load complete!" << std::endl;
}

void TPCCDB::loadWarehouses(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer w_ytd = Integer(INT_LENGTH, int(std::stof(row[1]) * 100), party_);// price is stored in cents
            Integer w_tax = Integer(INT_LENGTH, int(std::stof(row[2]) * 10000), party_);//convert float tax into integer
            warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer w_id = Integer(INT_LENGTH, 0, party_);
            Integer w_ytd = Integer(INT_LENGTH, 0, party_);
            Integer w_tax = Integer(INT_LENGTH, 0, party_);
            warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer w_id = Integer(INT_LENGTH, 0, party_);
            Integer w_ytd = Integer(INT_LENGTH, 0, party_);
            Integer w_tax = Integer(INT_LENGTH, 0, party_);
            warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer w_ytd = Integer(INT_LENGTH, int(std::stof(row[1]) * 100), party_);// price is stored in cents
            Integer w_tax = Integer(INT_LENGTH, int(std::stof(row[2]) * 10000), party_);
            warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Warehouse table load complete!" << std::endl;
}

void TPCCDB::loadStocks(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer s_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer s_i_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer s_quantity = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer s_ytd = Integer(INT_LENGTH, int(std::stof(row[3]) * 100), party_);
            Integer s_order_cnt = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            Integer s_remote_cnt = Integer(INT_LENGTH, std::stoi(row[5]), party_);
            stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer s_w_id = Integer(INT_LENGTH, 0, party_);
            Integer s_i_id = Integer(INT_LENGTH, 0, party_);
            Integer s_quantity = Integer(INT_LENGTH, 0, party_);
            Integer s_ytd = Integer(INT_LENGTH, 0, party_);
            Integer s_order_cnt = Integer(INT_LENGTH, 0, party_);
            Integer s_remote_cnt = Integer(INT_LENGTH, 0, party_);
            stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer s_w_id = Integer(INT_LENGTH, 0, party_);
            Integer s_i_id = Integer(INT_LENGTH, 0, party_);
            Integer s_quantity = Integer(INT_LENGTH, 0, party_);
            Integer s_ytd = Integer(INT_LENGTH, 0, party_);
            Integer s_order_cnt = Integer(INT_LENGTH, 0, party_);
            Integer s_remote_cnt = Integer(INT_LENGTH, 0, party_);
            stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer s_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer s_i_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer s_quantity = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer s_ytd = Integer(INT_LENGTH, int(std::stof(row[3]) * 100), party_);
            Integer s_order_cnt = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            Integer s_remote_cnt = Integer(INT_LENGTH, std::stoi(row[5]), party_);
            stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Stock table load complete!" << std::endl;
}

void TPCCDB::loadOrderLines(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer ol_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer ol_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer ol_o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer ol_number = Integer(INT_LENGTH, std::stoi(row[3]), party_);
            Integer ol_i_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            Integer ol_amount = Integer(INT_LENGTH, int(100 * std::stof(row[7])), party_);
            Integer ol_supply_w_id = Integer(INT_LENGTH, std::stoi(row[8]), party_);
            Integer ol_quantity = Integer(INT_LENGTH, std::stoi(row[9]), party_);
            orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer ol_w_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_d_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_o_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_number = Integer(INT_LENGTH, 0, party_);
            Integer ol_i_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_amount = Integer(INT_LENGTH, 0, party_);
            Integer ol_supply_w_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_quantity = Integer(INT_LENGTH, 0, party_);
            orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer ol_w_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_d_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_o_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_number = Integer(INT_LENGTH, 0, party_);
            Integer ol_i_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_amount = Integer(INT_LENGTH, 0, party_);
            Integer ol_supply_w_id = Integer(INT_LENGTH, 0, party_);
            Integer ol_quantity = Integer(INT_LENGTH, 0, party_);
            orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer ol_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer ol_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer ol_o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer ol_number = Integer(INT_LENGTH, std::stoi(row[3]), party_);
            Integer ol_i_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);
            Integer ol_amount = Integer(INT_LENGTH, int(100 * std::stof(row[7])), party_);
            Integer ol_supply_w_id = Integer(INT_LENGTH, std::stoi(row[8]), party_);
            Integer ol_quantity = Integer(INT_LENGTH, std::stoi(row[9]), party_);
            orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Orderline table load complete!" << std::endl;
}

void TPCCDB::loadNewOrders(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer no_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer no_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer no_o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer no_w_id = Integer(INT_LENGTH, 0, party_);
            Integer no_d_id = Integer(INT_LENGTH, 0, party_);
            Integer no_o_id = Integer(INT_LENGTH, 0, party_);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer no_w_id = Integer(INT_LENGTH, 0, party_);
            Integer no_d_id = Integer(INT_LENGTH, 0, party_);
            Integer no_o_id = Integer(INT_LENGTH, 0, party_);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer no_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer no_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer no_o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "NewOrder table load complete!" << std::endl;
}

void TPCCDB::loadOrders(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer o_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer o_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer o_c_id = Integer(INT_LENGTH, std::stoi(row[3]), party_);
            Integer o_ol_cnt = Integer(INT_LENGTH, std::stoi(row[5]), party_);
            orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer o_w_id = Integer(INT_LENGTH, 0, party_);
            Integer o_d_id = Integer(INT_LENGTH, 0, party_);
            Integer o_id = Integer(INT_LENGTH, 0, party_);
            Integer o_c_id = Integer(INT_LENGTH, 0, party_);
            Integer o_ol_cnt = Integer(INT_LENGTH, 0, party_);
            orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer o_w_id = Integer(INT_LENGTH, 0, party_);
            Integer o_d_id = Integer(INT_LENGTH, 0, party_);
            Integer o_id = Integer(INT_LENGTH, 0, party_);
            Integer o_c_id = Integer(INT_LENGTH, 0, party_);
            Integer o_ol_cnt = Integer(INT_LENGTH, 0, party_);
            orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer o_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer o_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);// price is stored in cents
            Integer o_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer o_c_id = Integer(INT_LENGTH, std::stoi(row[3]), party_);
            Integer o_ol_cnt = Integer(INT_LENGTH, std::stoi(row[5]), party_);
            orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Order table load complete!" << std::endl;
}

void TPCCDB::loadCustomers(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer c_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer c_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);
            Integer c_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer c_discount = Integer(INT_LENGTH, int(10000 * std::stof(row[3])), party_); //discount has four digits after the dot
            Integer c_balance = Integer(INT_LENGTH, 0, party_);//balance in cents
            Integer c_ytd_payment = Integer(INT_LENGTH, std::stoi(row[9]), party_);
            Integer c_payment_cnt = Integer(INT_LENGTH, std::stoi(row[10]), party_);
            Integer c_delivery_cnt = Integer(INT_LENGTH, std::stoi(row[11]), party_);
            customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer c_w_id = Integer(INT_LENGTH, 0, party_);
            Integer c_d_id = Integer(INT_LENGTH, 0, party_);
            Integer c_id = Integer(INT_LENGTH, 0, party_);
            Integer c_discount = Integer(INT_LENGTH, 0, party_);
            Integer c_balance = Integer(INT_LENGTH, 0, party_);
            Integer c_ytd_payment = Integer(INT_LENGTH, 0, party_);
            Integer c_payment_cnt = Integer(INT_LENGTH, 0, party_);
            Integer c_delivery_cnt = Integer(INT_LENGTH, 0, party_);
            customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer c_w_id = Integer(INT_LENGTH, 0, party_);
            Integer c_d_id = Integer(INT_LENGTH, 0, party_);
            Integer c_id = Integer(INT_LENGTH, 0, party_);
            Integer c_discount = Integer(INT_LENGTH, 0, party_);
            Integer c_balance = Integer(INT_LENGTH, 0, party_);
            Integer c_ytd_payment = Integer(INT_LENGTH, 0, party_);
            Integer c_payment_cnt = Integer(INT_LENGTH, 0, party_);
            Integer c_delivery_cnt = Integer(INT_LENGTH, 0, party_);
            customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer c_w_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer c_d_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);
            Integer c_id = Integer(INT_LENGTH, std::stoi(row[2]), party_);
            Integer c_discount = Integer(INT_LENGTH, int(10000 * std::stof(row[3])), party_); //discount has four digits after the dot
            Integer c_balance = Integer(INT_LENGTH, 0, party_);//balance in cents
            Integer c_ytd_payment = Integer(INT_LENGTH, std::stoi(row[9]), party_);
            Integer c_payment_cnt = Integer(INT_LENGTH, std::stoi(row[10]), party_);
            Integer c_delivery_cnt = Integer(INT_LENGTH, std::stoi(row[11]), party_);
            customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "Customer table load complete!" << std::endl;
}

void TPCCDB::loadDistricts(string fileAlice, string fileBob){

	NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    if(party_ == ALICE){
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer d_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer d_w_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);
            Integer d_ytd = Integer(INT_LENGTH, int(100 * std::stof(row[2])), party_);
            Integer d_tax = Integer(INT_LENGTH, int(10000 * std::stof(row[3])), party_); 
            Integer d_next_o_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);

            districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, party_));
        }
        for(int j = 0; j < lenBob; j++){
            //insert dummy tuples
            Integer d_id = Integer(INT_LENGTH, 0, party_);
            Integer d_w_id = Integer(INT_LENGTH, 0, party_);
            Integer d_ytd = Integer(INT_LENGTH, 0, party_);
            Integer d_tax = Integer(INT_LENGTH, 0, party_);
            Integer d_next_o_id = Integer(INT_LENGTH, 0, party_);

            districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, party_));
        }
        
    }else if(party_ == BOB){
        for(int j = 0; j < lenAlice; j++){
            //insert dummy tuples
            Integer d_id = Integer(INT_LENGTH, 0, party_);
            Integer d_w_id = Integer(INT_LENGTH, 0, party_);
            Integer d_ytd = Integer(INT_LENGTH, 0, party_);
            Integer d_tax = Integer(INT_LENGTH, 0, party_);
            Integer d_next_o_id = Integer(INT_LENGTH, 0, party_);

            districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, party_));
        }
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer d_id = Integer(INT_LENGTH, std::stoi(row[0]), party_);
            Integer d_w_id = Integer(INT_LENGTH, std::stoi(row[1]), party_);
            Integer d_ytd = Integer(INT_LENGTH, int(100 * std::stof(row[2])), party_);
            Integer d_tax = Integer(INT_LENGTH, int(10000 * std::stof(row[3])), party_); 
            Integer d_next_o_id = Integer(INT_LENGTH, std::stoi(row[4]), party_);

            districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, party_));
        }
    }else{
        std::cout << "Wrong party!" << std::endl;
    }
    io->flush();
    fa.close();
    fb.close();
    delete io;
    std::cout << "District table load complete!" << std::endl;
}