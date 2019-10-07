#pragma once
#include <typeinfo>
#include <emp-tool/emp-tool.h>
#include <emp-sh2pc/emp-sh2pc.h>
#include <emp-ot/emp-ot.h>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include<iostream>
#include<string>
#include<fstream>
#include<algorithm>
#include<sstream>
#include<dirent.h>
#include <assert.h>
using namespace std;
using namespace emp;

#define INT_LENGTH 32

class Tuple{
public:
    int cols; // number of columns
    vector<Integer>* elements; //column data

    Tuple(int c){
        cols = c;
        elements = new vector<Integer>(cols, Integer(INT_LENGTH, 0, PUBLIC));
    }

    Tuple(vector<Integer>& e){    
        cols = e.size();
        elements = new vector<Integer>(cols, Integer(INT_LENGTH, 0, PUBLIC));
        for(int i = 0; i < cols; i++){
            (*elements)[i] = e[i];
        }
    }

    ~Tuple(){
        delete[] elements;
    }

    Integer getElement(int index){
        assert(index >= 0 && index < cols);
        return elements[index];
    }

    void setElement(int index, Integer val){
        assert(index >= 0 && index < cols);
        elements[index] = val;
    }

    void If(const Bit& select, const Tuple& a) const{
        for(int i = 0; i < cols; i++){
            elements[i].If(select, a.elements[i]);
        }
    }
}

class Table{

//TODO : it is difficult to support insert/delete fields, which affects all tuples

    vector<Tuple*> tuples; // each Integer represent one element in a row
    vector<string> fields;
    map<string, int> offset; // column name to its offset in fields
    int rows; // number of tuples
    int cols;
    Table(vector<string>& f){
        rows = 0;
        cols = f.size();
        fields = f;
        for(int i = 0; i < fields.size(); i++){
            offset[fields[i]] = i;
        }
    }

    Table(vector<string>& f, vector<Tuple*> t){
        rows = t.size();
        cols = f.size();
        fields = f;
        for(int i = 0; i < fields.size(); i++){
            offset[fields[i]] = i;
        }
        tuples = t;
    }

    void insertTuple(Tuple* t){
        tuples.push_back(t);
        rows++;
    }

    Tuple* getTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields);

    bool removeTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields);


}

class TPCCDB{
public:
    int party_;
    int port_;
    Table* items_;
    Table* histories_;
    Table* warehouses_;
    Table* stocks_;
    Table* districts_;
    Table* customers_;
    Table* orders_;
    Table* orderlines_;
    Table* neworders_;

    TPCCDB(int party, int port);
    ~TPCCDB();

    void loadFromCSV(string fileAlice, string fileBob);

    //due to different csv pattern, it's hard to implement a general load csv API for all tables
    void loadItems(string fileAlice, string fileBob);
	void loadHistroy(string fileAlice, string fileBob);
	void loadWarehouses(string fileAlice, string fileBob);
	void loadStocks(string fileAlice, string fileBob);
	void loadDistricts(string fileAlice, string fileBob);
	void loadCustomers(string fileAlice, string fileBob);
	void loadOrders(string fileAlice, string fileBob);
	void loadOrderLines(string fileAlice, string fileBob);
	void loadNewOrders(string fileAlice, string fileBob);


    bool newOrder(Integer warehouse_id, Integer district_id, Integer customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    bool newOrderHome(Integer warehouse_id, Integer district_id, Integer customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    bool newOrderRemote(Integer home_warehouse, Integer remote_warehouse,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    vector<Integer> newOrderRemoteWarehouses(Integer home_warehouse, const std::vector<NewOrderItem>& items);

	void delivery(Integer warehouse_id, 
        std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo);


	void payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer customer_id, Integer h_amount, TPCCUndo** undo);

    void paymentHome(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer customer_id,  Integer h_amount, TPCCUndo** undo);
    void paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo);

	void internalPaymentRemote(Integer warehouse_id, Integer district_id, Customer* c,
        Integer h_amount,  TPCCUndo** undo);

	bool findAndValidateItems(const vector<NewOrderItem>& items, vector<Item*>* item_tuples);


}

TPCCUndo{
    public:
    ~TPCCUndo();

    // void save(Warehouse* w);
    // void save(District* d);
    // void save(Customer* c);
    // void save(Stock* s);
    // void save(Order* o);                                 
    // void save(OrderLine* o);

    // void inserted(const Order* o);
    // void inserted(const OrderLine* ol);
    // void inserted(const NewOrder* no);                                                                 
    // void inserted(const History* h);

    // void deleted(NewOrder* no);

    // Marks this undo buffer as applied. This prevents the destructor from deleting tuples
    // marked as deleted.
    // void applied();

    // typedef std::unordered_map<Warehouse*, Warehouse*> WarehouseMap;
    // const WarehouseMap& modified_warehouses() const { return modified_warehouses_; }

    // typedef std::unordered_map<District*, District*> DistrictMap;
    // const DistrictMap& modified_districts() const { return modified_districts_; }

    // typedef std::unordered_map<Customer*, Customer*> CustomerMap;
    // const CustomerMap& modified_customers() const { return modified_customers_; }

    // typedef std::unordered_map<Stock*, Stock*> StockMap;
    // const StockMap& modified_stock() const { return modified_stock_; }

    // typedef std::unordered_map<Order*, Order*> OrderMap;
    // const OrderMap& modified_orders() const { return modified_orders_; }

    // typedef std::unordered_map<OrderLine*, OrderLine*> OrderLineMap;
    // const OrderLineMap& modified_order_lines() const { return modified_order_lines_; }

    // typedef tpcc::Set<const Order*> OrderSet;
    // const OrderSet& inserted_orders() const { return inserted_orders_; }

    // typedef tpcc::Set<const OrderLine*> OrderLineSet;
    // const OrderLineSet& inserted_order_lines() const { return inserted_order_lines_; }

    // typedef tpcc::Set<const NewOrder*> NewOrderSet;
    // const NewOrderSet& inserted_new_orders() const { return inserted_new_orders_; }
    // typedef tpcc::Set<NewOrder*> NewOrderDeletedSet;
    // const NewOrderDeletedSet& deleted_new_orders() const { return deleted_new_orders_; }

    // typedef tpcc::Set<const History*> HistorySet;
    // const HistorySet& inserted_history() const { return inserted_history_; }

private:
    // WarehouseMap modified_warehouses_;
    // DistrictMap modified_districts_;
    // CustomerMap modified_customers_;
    // StockMap modified_stock_;
    // OrderMap modified_orders_;
    // OrderLineMap modified_order_lines_;

    // OrderSet inserted_orders_;
    // OrderLineSet inserted_order_lines_;
    // NewOrderSet inserted_new_orders_;
    // HistorySet inserted_history_;

    // NewOrderDeletedSet deleted_new_orders_;
}