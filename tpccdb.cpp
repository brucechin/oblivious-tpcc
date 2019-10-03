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

vector<std::string> split(std::string str, char delimiter){
    vector<std::string> internal;
    std::stringstream ss(str); // Turn the string into a stream.
    string result;
    
    // while(getline(ss, tok, delimiter)) {
    //     internal.push_back(tok);
    // }
    while(ss >> result){internal.push_back(result);}
    return internal;  
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
    //cout << "item size" << items.size()<<endl;
    for (int i = 0; i < items.size(); ++i) {
        //cout << "find item " << i << endl;
        (*item_tuples)[i] = findItem(items[i].ol_i_id);
        if ((*item_tuples)[i] == NULL) {
            cout << "wrong item!!!!"<<endl;
            return false;
        }
        //cout << "find it! " << endl;
    }
    return true;
}

bool TPCCDB::newOrder(Integer warehouse_id, Integer district_id, Integer customer_id,
        const std::vector<NewOrderItem>& items, TPCCUndo** undo) {
    // perform the home part
    cout<< "start new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    bool result = newOrderHome(warehouse_id, district_id, customer_id, items,  undo);
    if (!result) {
        return false;
    }

    // Process all remote warehouses
    vector<Integer> warehouses = newOrderRemoteWarehouses(warehouse_id, items);

    for(int i = 0; i < warehouses.size(); i++){
        result = newOrderRemote(warehouse_id, warehouses[i], items, undo);
    }
    //out<< "end new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    return true;
}


bool TPCCDB::newOrderRemote(Integer home_warehouse, Integer remote_warehouse,
        const vector<NewOrderItem>& items, TPCCUndo** undo) {
    // Validate all the items: needed so that we don't need to undo in order to execute this
    vector<Item*> item_tuples;
    if (!findAndValidateItems(items, &item_tuples)) {
        return false;
    }
    //cout << "finish item findandvalidate in remote" << endl;
    // We will not abort: allocate an undo buffer
    //allocateUndo(undo);

    for (int i = 0; i < items.size(); ++i) {
        // Skip items that don't belong to remote warehouse
        if((items[i].ol_supply_w_id != remote_warehouse).reveal<bool>(PUBLIC)){
            continue;
        }
        //cout <<"new order remote "<< i << endl;
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
    //District* d = new District();
    cout << "find district "<<endl;
    //d->print();

    //cout << "find customer " << customer_id.reveal<int>(PUBLIC)<<endl;
    Customer* c = findCustomer(warehouse_id, district_id, customer_id);

    // CHEAT: Validate all items to see if we will need to abort
    vector<Item*> item_tuples(items.size());
    if (!findAndValidateItems(items, &item_tuples)) {
        return false;
    }
    //cout << "finish item findandvalidate in home" << endl;

    // We will not abort: update the status and the database state, allocate an undo buffer
    //allocateUndo(undo);

    // Modify the order id to assign it
    // if (undo != NULL) {
    //     (*undo)->save(d);
    // }
    cout <<"district " << d->d_id.reveal<int>(PUBLIC) << " next order id :" <<d->d_next_o_id.reveal<int>(PUBLIC)<<endl;
    
    Integer tmp = d->d_next_o_id;
    Integer tmp2 = d->d_id;
    Integer tmp2_add = tmp2 + Integer(INT_LENGTH, 1, PUBLIC);
    Integer tmp_add = tmp + Integer(INT_LENGTH, 1, PUBLIC);
    d->d_next_o_id = d->d_next_o_id + Integer(INT_LENGTH, 1, PUBLIC);
    cout << "tmp2 :"<< tmp2.reveal<int>(PUBLIC) << "  increased to : " << tmp2_add.reveal<int>(PUBLIC) <<endl;
    cout << "tmp " << tmp.reveal<int>(PUBLIC) <<"  incresed to : " << tmp_add.reveal<int>(PUBLIC)<<endl;
    cout << "district "<< d->d_id.reveal<int>(PUBLIC) <<" next order id is increased to "<< d->d_next_o_id.reveal<int>(PUBLIC) <<endl;
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
        //cout <<"new order home "<< i << endl;
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
    cout << "start payment " << warehouse_id.reveal<int>(PUBLIC) << " "<< district_id.reveal<int>(PUBLIC) << " "<<customer_id.reveal<int>(PUBLIC) << endl;
    Customer* customer = findCustomer(c_warehouse_id, c_district_id, customer_id);
    paymentHome(warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount, undo);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
    cout << "end payment " << warehouse_id.reveal<int>(PUBLIC) << " "<< district_id.reveal<int>(PUBLIC) << " "<<customer_id.reveal<int>(PUBLIC) << endl;
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



void TPCCDB::delivery(Integer warehouse_id, 
        std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo) {
    //~ printf("delivery %d %d %s\n", warehouse_id, carrier_id, now);
    //allocateUndo(undo);
    orders->clear();
    cout << "start delivery " << warehouse_id.reveal<int>(PUBLIC) << endl;
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

    cout << "complete delivery " << warehouse_id.reveal<int>(PUBLIC) << endl;
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
        //cout << c_id.reveal<int>(PUBLIC) << " " << customers_[i]->c_id.reveal<int>(PUBLIC) << endl;
        bool flag = true;
        for(int i = 0; i < INT_LENGTH; i++){
            if((c_id[i] != customers_[i]->c_id[i]).reveal<bool>(PUBLIC)){
                flag = false;
                break;
            }
        }
        // if(true){
        //     cout << "same " << endl;
        // }else{
        //     cout << "diff" <<endl;
        // }
        //cout << w_id.reveal<int>(PUBLIC) << " " << customers_[i]->c_w_id.reveal<int>(PUBLIC) << " " << res1 <<endl;
        //cout << d_id.reveal<int>(PUBLIC) << " " << customers_[i]->c_d_id.reveal<int>(PUBLIC) << " " << res2 << endl;
        //cout << c_id.reveal<int>(PUBLIC) << " " << customers_[i]->c_id.reveal<int>(PUBLIC) << " " << res3 <<endl;
        //cout << endl;
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


Order* TPCCDB::findOrder(Integer w_id, Integer d_id, Integer o_id){

    int len = orders_.size();
    Order* res = NULL;
    for(int i = 0; i < len; i++){
        bool res1 = (orders_[i]->o_w_id == w_id).reveal<bool>(PUBLIC);
        bool res2 = (orders_[i]->o_d_id == d_id).reveal<bool>(PUBLIC);
        bool res3 = (orders_[i]->o_id == o_id).reveal<bool>(PUBLIC);
        if(res1 & res2 & res3){
            res = orders_[i]; // copy
        }
    }
    return res;
}


static string revealBinary(Integer &input, int length, int output_party) {
  cout << input.length << endl;
  bool * b = new bool[length];
  ProtocolExecution::prot_exec->reveal(b, output_party, (block *)input.bits,  length);
  char *bin = new char[length];

  for (int i=0; i<length; i++)
    bin[i] = (b[i] ? '1':'0');

  delete [] b;
  return string(bin);
  //return bin;
}

District* TPCCDB::findDistrict(Integer w_id, Integer d_id){
    int len = districts_.size();
    District* res = new District();
    // res->d_id = d_id;
    // res->d_w_id = w_id;
    cout << "next order id : " << districts_[0]->d_next_o_id.reveal<int>(PUBLIC) <<endl;
    
    // District output;
    // for(int i = 0; i < len; ++i) {
    //     output = If(districts_[i]->w_id == w_id & district_[i]->d_id == d_id, districts_[i], output)
    // }
    for(int i = 0; i < len; i++){
        //cout << "find district " << i << "th try " <<districts_[i]->d_w_id.reveal<int>(PUBLIC) << " " << w_id.reveal<int>(PUBLIC) << endl;
        bool res1 = (districts_[i]->d_w_id == w_id).reveal<bool>(PUBLIC);
        bool res2 = (districts_[i]->d_id == d_id).reveal<bool>(PUBLIC);
        

        Bit res3 = districts_[i]->d_w_id == w_id;
        Bit res4 = districts_[i]->d_id == d_id;
        cout << w_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_w_id.reveal<int>(PUBLIC) << " COMPARE : " << res3.reveal<bool>(PUBLIC) << "  " <<endl ;
        //Bit res(true);
        bool flag = true;
        for(int j = 0; j < INT_LENGTH; j++){
            // cout << res.reveal<bool>(PUBLIC) << " " << (w_id.bits[j] == districts_[i]->d_w_id.bits[j]).reveal<bool>(PUBLIC) <<" ";
            // res = res & (w_id.bits[j] == districts_[i]->d_w_id.bits[j]);
            // cout << res.reveal<bool>(PUBLIC) << endl;
            if((w_id[j] != districts_[i]->d_w_id[j]).reveal<bool>(PUBLIC) || (d_id[j] != districts_[i]->d_id[j]).reveal<bool>(PUBLIC)){
                flag = false;
                break;
            }
        }
        
        if(flag){
            res = districts_[i];
            cout << i << endl;
            cout << "next id : " << districts_[i]->d_next_o_id.reveal<int>(PUBLIC) <<endl;
            return res;
        }
        //cout << "warehouse id : " << w_id.reveal<int>(PUBLIC) << " "<< districts_[i]->d_w_id.reveal<int>(PUBLIC) << " compare res : " << (districts_[i]->d_w_id == w_id).reveal<bool>(PUBLIC) << " another way to compare : " << res3.reveal<bool>(PUBLIC)<<endl ;
        //cout << "district is : " << d_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_id.reveal<int>(PUBLIC) << " compare res : " << (districts_[i]->d_id == d_id).reveal<bool>(PUBLIC)<< " another way to compare : " << res4.reveal<bool>(PUBLIC) <<endl;
        //Bit tru = Bit(true, PUBLIC);
        //Bit fal = Bit(false, PUBLIC);

        // res->d_next_o_id = If(res3 & res4, districts_[i]->d_next_o_id, Integer(INT_LENGTH, 0, PUBLIC));
        // res->d_ytd = If(res3 & res4, districts_[i]->d_ytd, Integer(INT_LENGTH, 0, PUBLIC));
        // res->d_tax = If(res3 & res4, districts_[i]->d_tax, Integer(INT_LENGTH, 0, PUBLIC));


        //Bit res = If(res3 & res4, tru, fal);
        //cout << w_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_w_id.reveal<int>(PUBLIC) << " " << d_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_id.reveal<int>(PUBLIC) << " compare res : "<< res.reveal<bool>(PUBLIC)<<endl;
        // if(res1 && res2){
        //     cout <<"find the district : " << districts_[i]->d_w_id.reveal<int>(PUBLIC) <<" " << districts_[i]->d_id.reveal<int>(PUBLIC) <<endl;
        //     res = districts_[i];
        // }
    }
    return res;
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
    std::ifstream f(filename, ios::in);
    std::string line;
    while(std::getline(f, line, '\n')){
        res++;
    }
    f.close();
    return res;

}

void TPCCDB::loadItems(string fileAlice, string fileBob){
    string aliceHost = "127.0.0.1";
	//NetIO * io = new NetIO(party_==ALICE ? nullptr : aliceHost.c_str(), port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice, ios::in);
    std::ifstream fb(fileBob, ios::in);
    std::string line;
    cout << "start reading items " << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer i_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]) : 0, ALICE);
        Integer i_price = Integer(INT_LENGTH, (party_ == ALICE) ? int(std::stof(row[2]) * 100) : 0, ALICE);// price is stored in cents
        Integer i_im_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[4]) : 0, ALICE);
        items_.push_back(new Item(i_id, i_im_id, i_price, ALICE));
        //io->flush();
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer i_id = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[0]) : 0, BOB);
        Integer i_price = Integer(INT_LENGTH, (party_ == BOB) ?   int(std::stof(row[2]) * 100) : 0, BOB);// price is stored in cents
        Integer i_im_id = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[4]) : 0, BOB);
        items_.push_back(new Item(i_id, i_im_id, i_price, BOB));
        //io->flush();
    }
        
    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Item table load complete!" << std::endl;
}

void TPCCDB::loadWarehouses(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading warehouses " << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]) : 0, ALICE);
        Integer w_ytd = Integer(INT_LENGTH, (party_ == ALICE) ? int(std::stof(row[1]) * 100) : 0, ALICE);// price is stored in cents
        Integer w_tax = Integer(INT_LENGTH, (party_ == ALICE) ? int(std::stof(row[2]) * 10000) : 0, ALICE);//convert float tax into integer
        warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, ALICE));
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]) : 0, BOB);
        Integer w_ytd = Integer(INT_LENGTH, (party_ == BOB) ? int(std::stof(row[1]) * 100) : 0, BOB);// price is stored in cents
        Integer w_tax = Integer(INT_LENGTH, (party_ == BOB) ? int(std::stof(row[2]) * 10000) : 0, BOB);
        warehouses_.push_back(new Warehouse(w_id, w_ytd, w_tax, BOB));
    }
    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Warehouse table load complete!" << std::endl;
}

void TPCCDB::loadStocks(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading stocks " << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer s_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]) : 0, ALICE);
        Integer s_i_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]) : 0, ALICE);// price is stored in cents
        Integer s_quantity = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[2]) : 0, ALICE);
        Integer s_ytd = Integer(INT_LENGTH, (party_ == ALICE) ? int(std::stof(row[3]) * 100) : 0, ALICE);
        Integer s_order_cnt = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[4]) : 0, ALICE);
        Integer s_remote_cnt = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[5]) : 0, ALICE);
        stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, ALICE));
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer s_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]) : 0, BOB);
        Integer s_i_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[1]) : 0, BOB);// price is stored in cents
        Integer s_quantity = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[2]) : 0, BOB);
        Integer s_ytd = Integer(INT_LENGTH, (party_ == BOB) ? int(std::stof(row[3]) * 100) : 0, BOB);
        Integer s_order_cnt = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[4]) : 0, BOB);
        Integer s_remote_cnt = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[5]) : 0, BOB);
        stocks_.push_back(new Stock(s_i_id, s_w_id, s_order_cnt, s_remote_cnt, s_quantity, s_ytd, BOB));
    }

    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Stock table load complete!" << std::endl;
}

void TPCCDB::loadOrderLines(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading orderlines " << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer ol_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]): 0, ALICE);
        Integer ol_d_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]): 0, ALICE);// price is stored in cents
        Integer ol_o_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[2]): 0, ALICE);
        Integer ol_number = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[3]): 0, ALICE);
        Integer ol_i_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[4]): 0, ALICE);
        Integer ol_amount = Integer(INT_LENGTH, (party_ == ALICE) ? int(100 * std::stof(row[7])): 0, ALICE);
        Integer ol_supply_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[8]): 0, ALICE);
        Integer ol_quantity = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[9]): 0, ALICE);
        orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, ALICE));
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer ol_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]) : 0, BOB);
        Integer ol_d_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[1]): 0, BOB);// price is stored in cents
        Integer ol_o_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[2]): 0, BOB);
        Integer ol_number = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[3]): 0, BOB);
        Integer ol_i_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[4]): 0, BOB);
        Integer ol_amount = Integer(INT_LENGTH, (party_ == BOB) ? int(100 * std::stof(row[7])): 0, BOB);
        Integer ol_supply_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[8]): 0, BOB);
        Integer ol_quantity = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[9]): 0, BOB);
        orderlines_.push_back(new OrderLine(ol_w_id, ol_d_id, ol_o_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, BOB));
    }
    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Orderline table load complete!" << std::endl;
}

void TPCCDB::loadNewOrders(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading neworders " << endl;
    cout << lenAlice << " " << lenBob << endl;
        for(int i = 0; i < lenAlice; i++){
            std::getline(fa, line);
            vector<string> row = split(line, ' ');
            Integer no_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]) : 0, ALICE);
            Integer no_d_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]) : 0, ALICE);// price is stored in cents
            Integer no_o_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[2]) : 0, ALICE);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, ALICE));
        }
        cout << party_ <<" read half" << endl;
        for(int i = 0; i < lenBob; i++){
            std::getline(fb, line);
            vector<string> row = split(line, ' ');
            Integer no_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]): 0, BOB);
            Integer no_d_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[1]): 0, BOB);// price is stored in cents
            Integer no_o_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[2]): 0, BOB);

            neworders_.push_back(new NewOrder(no_w_id, no_d_id, no_o_id, BOB));
        }

    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "NewOrder table load complete!" << std::endl;
}

void TPCCDB::loadOrders(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer o_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]): 0, ALICE);
        Integer o_d_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]): 0, ALICE);// price is stored in cents
        Integer o_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[2]): 0, ALICE);
        Integer o_c_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[3]): 0, ALICE);
        Integer o_ol_cnt = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[5]): 0, ALICE);
        orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, ALICE));
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer o_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]) : 0, BOB);
        Integer o_d_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[1]) : 0, BOB);// price is stored in cents
        Integer o_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[2]) : 0, BOB);
        Integer o_c_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[3]) : 0, BOB);
        Integer o_ol_cnt = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[5]) : 0, BOB);
        orders_.push_back(new Order(o_id, o_w_id, o_d_id, o_c_id, o_ol_cnt, BOB));
    }

    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Order table load complete!" << std::endl;
}

void TPCCDB::loadCustomers(string fileAlice, string fileBob){

	//NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	//setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading customers" << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer c_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]) : 0, ALICE);
        Integer c_d_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]) : 0, ALICE);
        Integer c_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[2]) : 0, ALICE);
        Integer c_discount = Integer(INT_LENGTH, (party_ == ALICE) ? int(10000 * std::stof(row[3])) : 0, ALICE);//discount has four digits after the dot
        Integer c_balance = Integer(INT_LENGTH, (party_ == ALICE) ? 0 : 0, ALICE);//balance in cents
        Integer c_ytd_payment = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[9]) : 0, ALICE);
        Integer c_payment_cnt = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[10]) : 0, ALICE);
        Integer c_delivery_cnt = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[11]) : 0, ALICE);
        customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, ALICE));
    }
    cout << party_ <<" read half" << endl;

    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer c_w_id = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[0]): 0, BOB);
        Integer c_d_id = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[1]): 0, BOB);
        Integer c_id = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[2]): 0, BOB);
        Integer c_discount = Integer(INT_LENGTH, (party_ == BOB) ?  int(10000 * std::stof(row[3])): 0, BOB); //discount has four digits after the dot
        Integer c_balance = Integer(INT_LENGTH, (party_ == BOB) ?  0: 0, BOB);//balance in cents
        Integer c_ytd_payment = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[9]): 0, BOB);
        Integer c_payment_cnt = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[10]): 0, BOB);
        Integer c_delivery_cnt = Integer(INT_LENGTH, (party_ == BOB) ?  std::stoi(row[11]): 0, BOB);
        customers_.push_back(new Customer(c_id, c_d_id, c_w_id, c_payment_cnt, c_delivery_cnt, c_discount, c_balance, c_ytd_payment, BOB));
    }

    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "Customer table load complete!" << std::endl;
}

void TPCCDB::loadDistricts(string fileAlice, string fileBob){

	// NetIO * io = new NetIO(party_==ALICE ? nullptr : "127.0.0.1", port_);
	// setup_semi_honest(io, party_);
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    std::ifstream fa(fileAlice);
    std::ifstream fb(fileBob);
    //TODO : add code to //ASSERT that file stream open succress?
    std::string line;
    cout << "start reading districts " << endl;
    cout << lenAlice << " " << lenBob << endl;
    for(int i = 0; i < lenAlice; i++){
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        Integer d_w_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[0]): 0, ALICE);
        Integer d_id = Integer(INT_LENGTH, (party_ == ALICE) ? std::stoi(row[1]): 0, ALICE);
        Integer d_ytd = Integer(INT_LENGTH, (party_ == ALICE) ? int(100 * std::stof(row[2])): 0, ALICE);
        Integer d_tax = Integer(INT_LENGTH, (party_ == ALICE) ? int(10000 * std::stof(row[3])): 0, ALICE);
        Integer d_next_o_id = Integer(INT_LENGTH, (party_ == ALICE) ? 31 : 0, ALICE);
        // cout << d_next_o_id.reveal<int>(PUBLIC)  << " ";
        // d_next_o_id =  d_next_o_id + Integer(INT_LENGTH, 1, PUBLIC);
        // cout << d_next_o_id.reveal<int>(PUBLIC)<<endl;
        //cout <<"district " <<std::stoi(row[1]) <<" next order id is " << std::stoi(row[4]) << " " << d_next_o_id.reveal<int>(PUBLIC) << endl;
        // for(int i = 0; i < 100; i++){
        //     d_next_o_id = d_next_o_id + Integer(INT_LENGTH, 1, PUBLIC);
        //     cout << "d next order id : "<< d_next_o_id.reveal<int>(PUBLIC)<<endl;
        // }
        districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, ALICE));
    }
    cout << party_ <<" read half" << endl;
    for(int i = 0; i < lenBob; i++){
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        Integer d_w_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[0]): 0, BOB);
        Integer d_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[1]): 0, BOB);
        Integer d_ytd = Integer(INT_LENGTH, (party_ == BOB) ? int(100 * std::stof(row[2])): 0, BOB);
        Integer d_tax = Integer(INT_LENGTH, (party_ == BOB) ? int(10000 * std::stof(row[3])): 0, BOB);
        Integer d_next_o_id = Integer(INT_LENGTH, (party_ == BOB) ? std::stoi(row[4]): 0, BOB);

        districts_.push_back(new District(d_id, d_w_id, d_next_o_id, d_ytd, d_tax, BOB));
    }
    // for(int i = 0; i < districts_.size(); i++){
    //     cout << districts_[i]->d_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_w_id.reveal<int>(PUBLIC) << " " << districts_[i]->d_next_o_id.reveal<int>(PUBLIC) << endl;
    // }
    //io->flush();
    fa.close();
    fb.close();
    //delete io;
    std::cout << "District table load complete!" << std::endl;
}