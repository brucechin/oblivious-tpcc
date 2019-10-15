#include "tpccdb_tmp.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>
#include <chrono>
using namespace std;

void print(vector<int> vec){
    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }
    cout << endl;
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

// Allocates an undo buffer if needed, storing the pointer in *undo.
void allocateUndo(TPCCUndo** undo) {
    if (undo != nullptr && *undo == nullptr) {
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
    delete items_;
    delete histories_;
    delete orders_;
    delete stocks_;
    delete districts_;
    delete customers_;
    delete orderlines_;
    delete neworders_;
}


TPCCDB::TPCCDB(int party, int port) :
    party_(party),
    port_(port) {
        //initialize the fields name for each table
        items_ = new Table(vector<string>{"i_id", "i_price", "i_im_id"});
        warehouses_ = new Table(vector<string>{"w_id", "w_ytd", "w_tax"});
        stocks_ = new Table(vector<string>{"s_w_id", "s_i_id", "s_quantity", "s_ytd", "s_order_cnt", "s_remote_cnt"});
        orderlines_ = new Table(vector<string>{"ol_w_id", "ol_d_id", "ol_o_id", "ol_number", "ol_i_id", "ol_amount", "ol_supply_w_id", "ol_quantity"});
        neworders_ = new Table(vector<string>{"no_w_id", "no_d_id", "no_o_id"});
        orders_ = new Table(vector<string>{"o_w_id", "o_d_id", "o_id", "o_c_id", "o_ol_cnt"});
        customers_ = new Table(vector<string>{"c_w_id", "c_d_id", "c_id", "c_discount", "c_balance", "c_ytd_payment", "c_payment_cnt", "c_delivery_cnt"});
        districts_ = new Table(vector<string>{"d_w_id", "d_id", "d_ytd", "d_tax", "d_next_o_id"});
        histories_ = new Table(vector<string>{"h_w_id", "h_d_id", "h_c_w_id", "h_c_d_id", "h_c_id", "h_amount"});
}

void print(vector<Integer> vec){
    cout << "print keys : ";
    for(int i = 0; i < vec.size(); i++){
        cout << vec[i].reveal<int>(PUBLIC) << " ";
    }
    cout << endl;
}

vector<Tuple*> Table::getTuplesByPrimaryKeyBatch(vector<vector<Integer>> keys, const vector<string> keyFields){
    vector<Tuple*> output;
    vector<int> keysOffset;
    for(int i = 0; i < keyFields.size(); i++){
        keysOffset.push_back(offset[keyFields[i]]);
    }
    cout << "batch size : " << keys.size() << endl;
    for(int i = 0; i < rows; i++){
        
        Tuple* t = tuples[i];
        // Bit res = Bit(true, PUBLIC);
        // for(int j = 0; j < keys.size(); j++){
        //     res = res & (keys[j] == (*t->elements)[keysOffset[j]]);
        // }  
        // output->If(res, *t);
        
        for(int k = 0; k < keys.size(); k++){
            bool res = true;
            for(int j = 0; j < keys[k].size(); j++){
                //cout << t->elements[keysOffset[j]].reveal<int>(PUBLIC) << " ";
                res = res && ((keys[k][j] == t->elements[keysOffset[j]]).reveal<bool>(PUBLIC));
            }
            if(res){
                //cout << "find one : " << endl;
                output.push_back(t);
                keys.erase(keys.begin() + k);
                continue;
            }
        }
    }
    //cout << endl;
    return output;

}


Tuple* Table::getTupleByPrimaryKey(vector<Integer> keys, const vector<string> keyFields){
    Tuple* output = new Tuple(10);
    vector<int> keysOffset;
    //print(keys);
    //cout << res << endl;
    for(int i = 0; i < keyFields.size(); i++){
        keysOffset.push_back(offset[keyFields[i]]);
    }
    //cout << "table has " << rows << " rows" << endl;
    cout << std::this_thread::get_id() << "  reading" << endl;
    for(int i = 0; i < rows; i++){
        std::shared_lock lock(mutexes_[i]);
        Tuple* t = tuples[i];
        // Bit res = Bit(true, PUBLIC);
        // for(int j = 0; j < keys.size(); j++){
        //     res = res & (keys[j] == (*t->elements)[keysOffset[j]]);
        // }  
        // output->If(res, *t);
        bool res = true;
        
        // for(int j = 0; j < keys.size(); j++){
        //     //cout << t->elements[keysOffset[j]].reveal<int>(PUBLIC) << " ";
        //     res = res && (keys[j] == t->elements[keysOffset[j]]).reveal<bool>(PUBLIC);
        // }
        if(res){
            output = t;
        }
    }
    //cout << endl;
    return output;
}

bool Table::removeTupleByPrimaryKey(vector<Integer> keys, const vector<string> keyFields){
    //input keys and keyFields are corresponding
    
    vector<int> keysOffset;
    for(int i = 0; i < keyFields.size(); i++){
        keysOffset.push_back(offset[keyFields[i]]);
    }

    for(int i = 0; i < rows; i++){
        Tuple* t = tuples[i];
        Bit res = Bit(true, PUBLIC);
        for(int j = 0; j < keys.size(); j++){
            res = res & (keys[j] == t->elements[keysOffset[j]]);
        }

        if(res.reveal<bool>(PUBLIC)){
            //TODO : how to implement delete without revealing "res" ? 
            //delete this tuple and decrease the rows by one
        }
    }

    return true;
}

void Table::loadFromCSVBatcher(string fileAlice, string fileBob, vector<int> targetColumns, vector<int> scales, int party){
    //here is a generalized loading function from CSV file
    //targetColumns contains offsets of corresponding values we want to load into elements of each tuple
    //scales is used to convert some float values to int for future convenience
    
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    int numCol = targetColumns.size();
    rows = lenAlice + lenBob;
    assert(numCol == scales.size());
    std::ifstream fa(fileAlice, ios::in);
    std::ifstream fb(fileBob, ios::in);
    std::string line;
    // cout << "start reading items " << endl;
    // cout << fileAlice <<  " " << fileBob << endl;
    // cout << lenAlice << " " << lenBob <<endl;
    //print(targetColumns);
    Batcher batcherAlice, batcherBob;
    for(int i = 0; i < lenAlice; i++)
    {
        std::getline(fa, line);
        vector<string> row = split(line, ' ');
        for(int k = 0; k < numCol; k++){
            if(scales[k] == 1){
                batcherAlice.add<Integer>(INT_LENGTH, std::stoi(row[targetColumns[k]]));
            }else{
                batcherAlice.add<Integer>(INT_LENGTH, int(std::stof(row[targetColumns[k]]) * scales[k]));
            }
        }
    }

    for(int i = 0; i < lenBob; i++)
    {
        std::getline(fb, line);
        vector<string> row = split(line, ' ');
        for(int k = 0; k < numCol; k++){
            if(scales[k] == 1){
                batcherBob.add<Integer>(INT_LENGTH, std::stoi(row[targetColumns[k]]));
            }else{
                batcherBob.add<Integer>(INT_LENGTH, int(std::stof(row[targetColumns[k]]) * scales[k]));
            }
        }
    }

    batcherBob.make_semi_honest(BOB);
    batcherAlice.make_semi_honest(ALICE);

    for(int i = 0; i < lenAlice; i++){
        vector<Integer> ele;
        for(int k = 0; k < numCol; k++){
            ele.push_back(batcherAlice.next<Integer>());
        }
        Tuple* t = new Tuple(ele);
        tuples.push_back(t);
    }

    for(int i = 0; i < lenBob; i++){
        vector<Integer> ele;
        for(int k = 0; k < numCol; k++){
            ele.push_back(batcherBob.next<Integer>());
        }
        Tuple* t = new Tuple(ele);
        tuples.push_back(t);
    }


    fa.close();
    fb.close();
}

void Table::loadFromCSV(string fileAlice, string fileBob, vector<int> targetColumns, vector<int> scales, int party){
    //here is a generalized loading function from CSV file
    //targetColumns contains offsets of corresponding values we want to load into elements of each tuple
    //scales is used to convert some float values to int for future convenience
    
    int lenAlice = numOfRows(fileAlice);
    int lenBob = numOfRows(fileBob);
    int numCol = targetColumns.size();
    rows = lenAlice + lenBob;
    assert(numCol == scales.size());
    std::ifstream fa(fileAlice, ios::in);
    std::ifstream fb(fileBob, ios::in);
    std::string line;
    cout << "start reading items " << endl;
    cout << fileAlice <<  " " << fileBob << endl;
    cout << lenAlice << " " << lenBob <<endl;
    for(int i = 0; i < lenAlice + lenBob; i++){
        std::getline(i < lenAlice ? fa : fb, line);
        vector<string> row = split(line, ' ');
        vector<Integer> ele;
        //TODO : batch encryption for better performance
        for(int k = 0; k < numCol; k++){
            if(scales[k] > 1){
                ele.push_back(Integer(INT_LENGTH, (party == (i < lenAlice ? ALICE : BOB)) ? stoi(row[targetColumns[k]]) : 0, (i < lenAlice ? ALICE : BOB)));
            }else{
                //we need to scale the float to int by multiply the scale factor
                ele.push_back(Integer(INT_LENGTH, (party == (i < lenAlice ? ALICE : BOB)) ? int(stof(row[targetColumns[k]]) * scales[k]) : 0, (i < lenAlice ? ALICE : BOB)));
            }
        }
        //cout << ele[0].reveal<int>(PUBLIC) << endl;
        Tuple* t = new Tuple(ele);
        tuples.push_back(t);
    }
    fa.close();
    fb.close();
}

void TPCCDB::loadFromCSV(string fileAlice, string fileBob){
    items_->loadFromCSVBatcher(dataFilePath + "item" + fileAlice, dataFilePath + "item" + fileBob, vector<int>({0, 2, 4}),vector<int>({1, 100, 1}), party_);
    warehouses_->loadFromCSVBatcher(dataFilePath + "warehouse" + fileAlice, dataFilePath + "warehouse" + fileBob, vector<int>({0, 1 ,2}), vector<int>({1, 100, 10000}), party_);
    stocks_->loadFromCSVBatcher(dataFilePath + "stock" + fileAlice, dataFilePath + "stock" + fileBob, vector<int>({0, 1, 2, 3, 4, 5}), vector<int>({1, 1, 1, 100, 1, 1}), party_);
    districts_->loadFromCSVBatcher(dataFilePath + "district" + fileAlice, dataFilePath + "district" + fileBob, vector<int>({0, 1, 2, 3, 4}), vector<int>({1, 1, 1, 1, 1}), party_);
    customers_->loadFromCSVBatcher(dataFilePath + "customer" + fileAlice, dataFilePath + "customer" + fileBob, vector<int>({0, 1, 2, 3, 8, 9, 10, 11}), vector<int>({1, 1, 1, 10000, 100, 1, 1, 1}), party_);
    orders_->loadFromCSVBatcher(dataFilePath + "oorder" + fileAlice, dataFilePath + "oorder" + fileBob, vector<int>({0, 1, 2, 3, 5}), vector<int>({1, 1, 1, 1, 1}), party_);
    orderlines_->loadFromCSVBatcher(dataFilePath + "order_line" + fileAlice, dataFilePath + "order_line" + fileBob, vector<int>({0, 1, 2, 3, 4, 6, 7, 8}), vector<int>({1, 1, 1, 1, 1, 100, 1, 1}), party_);
    neworders_->loadFromCSVBatcher(dataFilePath + "new_order" + fileAlice, dataFilePath + "new_order" + fileBob, vector<int>({0, 1, 2}), vector<int>({1, 1, 1}), party_);
    cout << "all tables loaded completely" << endl;
}

bool TPCCDB::findAndValidateItemsBatch(const vector<NewOrderItem*>& items,
        vector<Item*>& item_tuples) {
    // CHEAT: Validate all items to see if we will need to abort
    cout << "item size " << items.size()<<endl;
    std::vector<vector<Integer>> itemKeys;
    for(int i = 0; i < items.size(); i++){
        itemKeys.push_back(vector<Integer>({items[i]->getElement(1)}));
    }
    vector<Item*> out = items_->getTuplesByPrimaryKeyBatch(itemKeys, itemPrimaryKeyNames_);
    for(int i = 0; i < out.size(); i++){
        item_tuples[i] = out[i];
    }
    return true;
}


bool TPCCDB::findAndValidateItems(const vector<NewOrderItem*>& items,
        vector<Item*>& item_tuples) {
    // CHEAT: Validate all items to see if we will need to abort
    item_tuples.resize(items.size());
    //cout << "item size" << items.size()<<endl;

    for (int i = 0; i < items.size(); ++i) {
        item_tuples[i] = items_->getTupleByPrimaryKey(vector<Integer>({items[i]->getElement(1)}), itemPrimaryKeyNames_);
        if (item_tuples[i] == nullptr) {
            cout << "wrong item!!!!"<<endl;
            return false;
        }
    }
    return true;
}

bool TPCCDB::newOrder(Integer warehouse_id, Integer district_id, Integer customer_id,
        const std::vector<NewOrderItem*>& items, TPCCUndo** undo) {
    // perform the home part
    cout<< "start new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    bool result = newOrderHome(warehouse_id, district_id, customer_id, items, undo);
    cout << "end new order" <<endl;
    if (!result) {
        return false;
    }

    // Process all remote warehouses
    // vector<Integer> warehouses = newOrderRemoteWarehouses(warehouse_id, items);

    // for(int i = 0; i < warehouses.size(); i++){
    //     result = newOrderRemote(warehouse_id, warehouses[i], items, undo);
    // }
    cout<< "end new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    return true;
}

bool TPCCDB::newOrderBatch(Integer warehouse_id, Integer district_id, Integer customer_id,
        const std::vector<NewOrderItem*>& items, TPCCUndo** undo) {
    // perform the home part
    cout<< "start new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    bool result = newOrderHomeBatch(warehouse_id, district_id, customer_id, items, undo);
    if (!result) {
        return false;
    }

    // Process all remote warehouses
    // vector<Integer> warehouses = newOrderRemoteWarehouses(warehouse_id, items);

    // for(int i = 0; i < warehouses.size(); i++){
    //     result = newOrderRemote(warehouse_id, warehouses[i], items, undo);
    // }
    cout<< "end new order" << warehouse_id.reveal<int>(PUBLIC) << " " << district_id.reveal<int>(PUBLIC) << " " << customer_id.reveal<int>(PUBLIC) << endl;
    return true;
}

bool TPCCDB::newOrderRemoteBatch(Integer home_warehouse, Integer remote_warehouse,
        const vector<NewOrderItem*>& items, TPCCUndo** undo) {
            return true;
}

bool TPCCDB::newOrderRemote(Integer home_warehouse, Integer remote_warehouse,
        const vector<NewOrderItem*>& items, TPCCUndo** undo) {
    // Validate all the items: needed so that we don't need to undo in order to execute this
    vector<Item*> item_tuples;
    if (!findAndValidateItems(items, item_tuples)) {
        return false;
    }
    cout << "find and validate items done" << endl;

    // We will not abort: allocate an undo buffer
    //allocateUndo(undo);

    for (int i = 0; i < items.size(); ++i) {
        // Skip items that don't belong to remote warehouse
        if((items[i]->getElement(1) != remote_warehouse).reveal<bool>(PUBLIC)){
            continue;
        }
        // update stock
        Stock* stock = stocks_->getTupleByPrimaryKey(vector<Integer>({items[i]->getElement(0), items[i]->getElement(1)}), stockPrimaryKeyNames_);
        // if (undo != nullptr) {
        //     (*undo)->save(stock);
        // }

        Integer stock_quantity = stock->getElement(stocks_->offset["s_quantity"]);
        Integer item_ol_quantity = items[i]->getElement(items_->offset["ol_quantity"]);


        Bit cmp = (stock_quantity >= item_ol_quantity + Integer(INT_LENGTH, 10, PUBLIC));
        stock->setElement(stocks_->offset["s_quantity"], stock_quantity - item_ol_quantity + Integer(INT_LENGTH, 91, PUBLIC));
        stock->If(stocks_->offset["s_quantity"], cmp, stock_quantity - item_ol_quantity);

        stock->setElement(stocks_->offset["s_ytd"], stock->getElement(stocks_->offset["s_ytd"]) + item_tuples[i]->getElement(items_->offset["i_price"]) * items[i]->getElement(2));
        stock->setElement(stocks_->offset["s_order_cnt"], stock->getElement(stocks_->offset["s_order_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));

        //newOrderHome calls newOrderRemote, so this is needed
        if ((items[i]->getElement(0) != home_warehouse).reveal<bool>(PUBLIC)) {
            stock->setElement(stocks_->offset["s_remote_cnt"], stock->getElement(stocks_->offset["s_remote_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));
        }
    }

    return true;
}

bool TPCCDB::newOrderHomeBatch(Integer warehouse_id, Integer district_id, Integer customer_id,
        const vector<NewOrderItem*>& items, TPCCUndo** undo) {

    // read those values first
    auto allStart = std::chrono::high_resolution_clock::now();
    District* d = districts_->getTupleByPrimaryKey(vector<Integer>({warehouse_id, district_id}), districtPrimaryKeyNames_);
    
    //cout << "find district "<< d->getElement(1).reveal<int>(PUBLIC) <<endl;

    
    Customer* c = customers_->getTupleByPrimaryKey(vector<Integer>({warehouse_id, district_id, customer_id}), customerPrimaryKeyNames_);
    // CHEAT: Validate all items to see if we will need to abort

    auto start = std::chrono::high_resolution_clock::now();
    vector<Item*> item_tuples;
    for(int i = 0; i < items.size(); i++){
        item_tuples.push_back(new Item(3));
    }
    if (!findAndValidateItemsBatch(items, item_tuples)) {
        return false;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "find and validate items : " <<elapsed.count() <<endl;
    //cout << "find and validate items done" << endl;

    // We will not abort: update the status and the database state, allocate an undo buffer
    //allocateUndo(undo);

    // Modify the order id to assign it
    // if (undo != nullptr) {
    //     (*undo)->save(d);
    // }
    Integer tmp = d->getElement(districts_->offset["d_next_o_id"]) + Integer(INT_LENGTH, 1, PUBLIC);
    //cout << "next order id is increased to : " <<tmp.reveal<int>(PUBLIC) << endl;
    d->setElement(districts_->offset["d_next_o_id"], d->getElement(districts_->offset["d_next_o_id"]) + Integer(INT_LENGTH, 1, PUBLIC));
    Warehouse* w = warehouses_->getTupleByPrimaryKey(vector<Integer>({warehouse_id}), warehousePrimaryKeyNames_);
    Tuple* order = new Tuple(vector<Integer>({warehouse_id, district_id, d->getElement(districts_->offset["d_next_o_id"]), customer_id, Integer(INT_LENGTH, items.size(), PUBLIC)}));
    orders_->insertTuple(order);
    Tuple* neworder = new Tuple(vector<Integer>({warehouse_id, district_id, order->getElement(orders_->offset["o_id"])}));
    neworders_->insertTuple(neworder);
    // Order* o = insertOrder(order);
    // NewOrder* no = insertNewOrder(warehouse_id, district_id, d->d_next_o_id);
    // if (undo != nullptr) {
    //     (*undo)->inserted(o);
    //     (*undo)->inserted(no);
    // }

    std::vector<vector<Integer>> stockKeys;
    for(int i = 0; i < items.size(); i++)
    {
        stockKeys.push_back(vector<Integer>({items[i]->getElement(0), items[i]->getElement(1)}));
    }
    start = std::chrono::high_resolution_clock::now();
    vector<Stock*> targetStocks = stocks_->getTuplesByPrimaryKeyBatch(stockKeys, stockPrimaryKeyNames_);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "stock find time : " << elapsed.count() << endl;
    for(int i = 0; i < targetStocks.size(); i++){
        targetStocks[i]->setElement(stocks_->offset["s_quantity"], targetStocks[i]->getElement(stocks_->offset["s_quantity"]) - items[i]->getElement(2));
        targetStocks[i]->setElement(stocks_->offset["s_order_cnt"], targetStocks[i]->getElement(stocks_->offset["s_order_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));
        targetStocks[i]->setElement(stocks_->offset["s_ytd"], targetStocks[i]->getElement(stocks_->offset["s_ytd"]) + items[i]->getElement(2) * item_tuples[i]->getElement(items_->offset["i_price"]));
        Tuple* orderline = new Tuple(vector<Integer>({warehouse_id, district_id, order->getElement(orders_->offset["o_id"]),
                                                        Integer(INT_LENGTH, i + 1, PUBLIC), items[i]->getElement(1), items[i]->getElement(2),
                                                        items[i]->getElement(0), items[i]->getElement(2) * item_tuples[i]->getElement(items_->offset["i_price"])}));
        orderlines_->insertTuple(orderline);
        //OrderLine* ol = insertOrderLine(line);
        // if (undo != nullptr) {
        //     (*undo)->inserted(ol);
        // }
    }

    
    // Perform the "remote" part for this warehouse
    //bool result = newOrderRemote(warehouse_id, warehouse_id, items,  undo);
    auto allEnd = std::chrono::high_resolution_clock::now();
    elapsed = allEnd - allStart;
    cout << "total new order home time : " << elapsed.count() << endl;
    return true;
}

bool TPCCDB::newOrderHome(Integer warehouse_id, Integer district_id, Integer customer_id,
        const vector<NewOrderItem*>& items, TPCCUndo** undo) {
    cout << "start new order home " <<endl;
    // read those values first
    District* d = districts_->getTupleByPrimaryKey(vector<Integer>({warehouse_id, district_id}), districtPrimaryKeyNames_);
    
    //cout << "find district "<< d->getElement(1).reveal<int>(PUBLIC) <<endl;
    //d->print();

    
    Customer* c = customers_->getTupleByPrimaryKey(vector<Integer>({warehouse_id, district_id, customer_id}), customerPrimaryKeyNames_);
    //cout << "find customer " << customer_id.reveal<int>(PUBLIC)<<endl;
    // CHEAT: Validate all items to see if we will need to abort
    //TODO here may be the segmentation fault

    vector<Item*> item_tuples;
    for(int i = 0; i < items.size(); i++){
        item_tuples.push_back(new Item(3));
    }
    if (!findAndValidateItems(items, item_tuples)) {
        return false;
    }
    cout << "find and validate items done" << endl;

    // We will not abort: update the status and the database state, allocate an undo buffer
    //allocateUndo(undo);

    // Modify the order id to assign it
    // if (undo != nullptr) {
    //     (*undo)->save(d);
    // }
    //cout <<"next order id offset is :" <<districts_->offset["d_next_o_id"] << endl;
    //cout << "district Tuple cols : " << d->cols <<endl;
    //cout << "next oreder id is : " << d->getElement(districts_->offset["d_next_o_id"]).reveal<int>(PUBLIC) <<endl;
    Integer tmp = d->getElement(districts_->offset["d_next_o_id"]) + Integer(INT_LENGTH, 1, PUBLIC);
    //cout << "next order id is increased to : " <<tmp.reveal<int>(PUBLIC) << endl;
    d->setElement(districts_->offset["d_next_o_id"], d->getElement(districts_->offset["d_next_o_id"]) + Integer(INT_LENGTH, 1, PUBLIC));
    cout << "next order id increased" <<endl;
    Warehouse* w = warehouses_->getTupleByPrimaryKey(vector<Integer>({warehouse_id}), warehousePrimaryKeyNames_);
    cout << "warehouse searched" <<endl;
    Tuple* order = new Tuple(vector<Integer>({warehouse_id, district_id, d->getElement(districts_->offset["d_next_o_id"]), customer_id, Integer(INT_LENGTH, items.size(), PUBLIC)}));
    orders_->insertTuple(order);
    cout << "order inserted" <<endl;
    Tuple* neworder = new Tuple(vector<Integer>({warehouse_id, district_id, order->getElement(orders_->offset["o_id"])}));
    neworders_->insertTuple(neworder);
    cout << "new order inserted" <<endl;
    // Order* o = insertOrder(order);
    // NewOrder* no = insertNewOrder(warehouse_id, district_id, d->d_next_o_id);
    // if (undo != nullptr) {
    //     (*undo)->inserted(o);
    //     (*undo)->inserted(no);
    // }

    for (int i = 0; i < items.size(); ++i) {
        //cout << items[i]->getElement(0).reveal<int>(PUBLIC) << " " <<items[i]->getElement(1).reveal<int>(PUBLIC)<<endl;
        Stock* stock = stocks_->getTupleByPrimaryKey(vector<Integer>({items[i]->getElement(0), items[i]->getElement(1)}), stockPrimaryKeyNames_);
        cout << "find stock : " << stock->getElement(0).reveal<int>(PUBLIC) << " " << stock->getElement(1).reveal<int>(PUBLIC) << endl;
        stock->setElement(stocks_->offset["s_quantity"], stock->getElement(stocks_->offset["s_quantity"]) - items[i]->getElement(2));
        stock->setElement(stocks_->offset["s_order_cnt"], stock->getElement(stocks_->offset["s_order_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));
        stock->setElement(stocks_->offset["s_ytd"], stock->getElement(stocks_->offset["s_ytd"]) + items[i]->getElement(2) * item_tuples[i]->getElement(items_->offset["i_price"]));
        Tuple* orderline = new Tuple(vector<Integer>({warehouse_id, district_id, order->getElement(orders_->offset["o_id"]),
                                                        Integer(INT_LENGTH, i + 1, PUBLIC), items[i]->getElement(1), items[i]->getElement(2),
                                                        items[i]->getElement(0), items[i]->getElement(2) * item_tuples[i]->getElement(items_->offset["i_price"])}));
        orderlines_->insertTuple(orderline);
        //OrderLine* ol = insertOrderLine(line);
        // if (undo != nullptr) {
        //     (*undo)->inserted(ol);
        // }
        cout << "orderline inserted" <<endl;
    }
    
    // Perform the "remote" part for this warehouse
    //bool result = newOrderRemote(warehouse_id, warehouse_id, items,  undo);

    return true;
}

vector<Integer> TPCCDB::newOrderRemoteWarehouses(Integer home_warehouse, const std::vector<NewOrderItem*>& items){
    vector<Integer> result;
    for (size_t i = 0; i < items.size(); ++i) {
        Integer supplyID = items[i]->getElement(0);
        if ((supplyID != home_warehouse).reveal<bool>(PUBLIC)) {
            result.push_back(supplyID);
        }
    }
    return result;
}


void TPCCDB::delivery(Integer warehouse_id, 
    std::vector<DeliveryOrderInfo*>* orders, TPCCUndo** undo){
        //allocateUndo(undo);
    orders->clear();
    cout << "start delivery " << warehouse_id.reveal<int>(PUBLIC) << endl;
    for (int32_t index = 1; index <= TPCCDB::DISTRICT_PER_WAREHOUSE; ++index) {
        //TODO : my implementation is not right. I do not find the lowest bound numbered new order according to warehouse id and district id
        //TODO : after finishing it, the new order should be removed from neworders_
        // Find and remove the lowest numbered order for the district
        // int64_t key = makeNewOrderKey(warehouse_id, d_id, 1);
        // NewOrderMap::iterator iterator = neworders_.lower_bound(key);
        // NewOrder* neworder = NULL;
        // if (iterator != neworders_.end()) {
        //     neworder = iterator->second;
        //     //ASSERT(neworder != NULL);
        // }
        Integer d_id = Integer(INT_LENGTH, index, PUBLIC);
        //NewOrder* neworder = neworders_->tuples[0];
        NewOrder* neworder = neworders_->getTupleByPrimaryKey(vector<Integer>{warehouse_id, d_id, Integer(INT_LENGTH, 1, PUBLIC)}, neworderPrimaryKeyNames_);
        if (neworder == NULL || (neworder->getElement(neworders_->offset["no_d_id"]) != d_id).reveal<bool>(PUBLIC) || (neworder->getElement(neworders_->offset["no_w_id"]) != warehouse_id).reveal<bool>(PUBLIC)) {
            // No orders for this district
            // TODO: 2.7.4.2: If this occurs in max(1%, 1) of transactions, report it (???)
            continue;
        }
        //ASSERT(neworder->no_d_id == d_id && neworder->no_w_id == warehouse_id);
        Integer o_id = neworder->getElement(neworders_->offset["no_o_id"]);
        
        //neworders_.erase(iterator);
        // if (undo != NULL) {
        //     (*undo)->deleted(neworder);
        // } else {
        //     delete neworder;
        // }

        DeliveryOrderInfo* order = new DeliveryOrderInfo(2);
        order->setElement(0, d_id);
        order->setElement(1, o_id);
        orders->push_back(order);

        //Order* o = orders_->getTupleByPrimaryKey(vector<Integer>{warehouse_id, d_id, o_id}, orderPrimaryKeyNames_);

        //ASSERT(o->o_carrier_id == Order::NULL_CARRIER_ID);
        // if (undo != NULL) {
        //     (*undo)->save(o);
        // }

        // Integer total = Integer(INT_LENGTH, 0, PUBLIC);
        // Integer i = Integer(INT_LENGTH, 1, PUBLIC);
        // while((i <= o->getElement(orders_->offset["o_ol_cnt"])).reveal<bool>(PUBLIC)){
        //     i = i + Integer(INT_LENGTH, 1, PUBLIC);
        //     OrderLine* line = orderlines_->getTupleByPrimaryKey(vector<Integer>{warehouse_id, d_id, o_id, i}, orderlinePrimaryKeyNames_);
        //     // if (undo != NULL) {
        //     //     (*undo)->save(line);
        //     // }
            
        //     total = total + line->getElement(orderlines_->offset["ol_amount"]);
        // }
        // Customer* c = customers_->getTupleByPrimaryKey(vector<Integer>{warehouse_id, d_id, o->getElement(orders_->offset["o_c_id"])}, customerPrimaryKeyNames_);
        // // if (undo != NULL) {
        // //     (*undo)->save(c);
        // // }
        // c->setElement(customers_->offset["c_balance"], c->getElement(customers_->offset["c_balance"]) + total);
        // c->setElement(customers_->offset["c_delivery_cnt"], c->getElement(customers_->offset["c_delivery_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));

    }

    cout << "complete delivery " << warehouse_id.reveal<int>(PUBLIC) << endl;
}


void TPCCDB::payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id, Integer h_amount, TPCCUndo** undo){

    cout << "start payment " << warehouse_id.reveal<int>(PUBLIC) << " "<< district_id.reveal<int>(PUBLIC) << " "<<customer_id.reveal<int>(PUBLIC) << endl;
    Customer* customer = customers_->getTupleByPrimaryKey(vector<Integer>{c_warehouse_id, c_district_id, customer_id}, customerPrimaryKeyNames_);
    paymentHome(warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount, undo);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
    cout << "end payment " << warehouse_id.reveal<int>(PUBLIC) << " "<< district_id.reveal<int>(PUBLIC) << " "<<customer_id.reveal<int>(PUBLIC) << endl;

}

void TPCCDB::paymentHome(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
        Integer c_district_id, Integer customer_id,  Integer h_amount, TPCCUndo** undo){
        
        Warehouse* w = warehouses_->getTupleByPrimaryKey(vector<Integer>{warehouse_id}, warehousePrimaryKeyNames_);
        // if (undo != NULL) {
        //     //allocateUndo(undo);
        //     (*undo)->save(w);
        // }
        w->setElement(warehouses_->offset["w_ytd"], w->getElement(warehouses_->offset["w_ytd"]) + h_amount);
        District* d = districts_->getTupleByPrimaryKey(vector<Integer>{warehouse_id, district_id}, districtPrimaryKeyNames_);
        // if (undo != NULL) {
        //     (*undo)->save(d);
        // }

        // Insert the line into the history table
        History* h = new History(vector<Integer>{warehouse_id, district_id, c_warehouse_id, c_district_id, customer_id, h_amount});

        histories_->insertTuple(h);
        // History* history = insertHistory(h);
        // if (undo != NULL) {
        //     (*undo)->inserted(history);
        // }

    }
void TPCCDB::paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
    Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo){
    Customer* customer = customers_->getTupleByPrimaryKey(vector<Integer>{c_warehouse_id, c_district_id, c_id}, customerPrimaryKeyNames_);
    internalPaymentRemote(warehouse_id, district_id, customer, h_amount, undo);
}

void TPCCDB::internalPaymentRemote(Integer warehouse_id, Integer district_id, Customer* c,
    Integer h_amount,  TPCCUndo** undo){

    // if (undo != NULL) {
    //     //allocateUndo(undo);
    //     (*undo)->save(c);
    // }
    c->setElement(customers_->offset["c_balance"], c->getElement(customers_->offset["c_balance"]) - h_amount);
    c->setElement(customers_->offset["c_ytd_payment"], c->getElement(customers_->offset["c_ytd_payment"]) + h_amount);
    c->setElement(customers_->offset["c_payment_cnt"], c->getElement(customers_->offset["c_payment_cnt"]) + Integer(INT_LENGTH, 1, PUBLIC));
    // c->c_balance = c->c_balance - h_amount;
    // c->c_ytd_payment = c->c_ytd_payment + h_amount;
    // c->c_payment_cnt = c->c_payment_cnt + Integer(INT_LENGTH, 1, PUBLIC);

}


    