#include "tpccdb_tmp.h"
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

Tuple Table::getTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields){
    Tuple output(cols);
    vector<int> keysOffset;
    for(int i = 0; i < keyFields.size(); i++){
        keysOffset.push_back(offset[keyFields[i]]);
    }
    for(int i = 0; i < rows; i++){
        Tuple* t = tuples[i];
        Bit res = Bit(true, PUBLIC);
        for(int j = 0; j < keys.size(); j++){
            res = res & (keys[j] == (*t->elements)[keysOffset[j]]);
        }  

        output.If(res, *t);
}

bool Table::removeTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields){
    //input keys and keyFields are corresponding
    
    vector<int> keysOffset;
    for(int i = 0; i < keyFields.size(); i++){
        keysOffset.push_back(offset[keyFields[i]]);
    }

    for(int i = 0; i < rows; i++){
        Tuple* t = tuples[i];
        Bit res = Bit(true, PUBLIC);
        for(int j = 0; j < keys.size(); j++){
            res = res & (keys[j] == (*t->elements)[keysOffset[j]]);
        }

        if(res.reveal<bool>(PUBLIC)){
            //TODO : how to implement delete without revealing "res" ? 
            //delete this tuple and decrease the rows by one
        }
    }

    return true;
}
