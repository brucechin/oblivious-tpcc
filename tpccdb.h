#pragma once
#include "assert.h"
#include "btree.h"
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#define INT_LENGTH = 32;
#define FLOAT32_LENGTH = 32;

namespace tpcc {
// was used to select between various non-standard implementations: now use std
template <typename T>
class Set : public std::unordered_set<T> {};
}

void writeInteger(Integer& target, Integer& data, int t_begin, int d_begin, int len){
	assert(t_begin + len < target.length);
	assert(d_begin + len < data.length);
	for(int i = 0; i < len; i++){
		target[t_begin + i] = data[d_begin + i]; // copy data Bits to override target Bits
	}
}

void readInteger(Integer& target, Integer& data, int t_begin, int d_begin, int len){
	assert(t_begin >= 0 && d_begin >= 0);
	assert(t_begin + len < target.length);
	assert(d_begin + len < data.length);
	for(int i = 0; i < len; i++){
		data[t_begin + i] = target[d_begin + i]; 
	}
}

void writeFloat32(Integer& target, Float32& data, int t_begin, int d_begin, int len){
	assert(t_begin >= 0 && d_begin >= 0);
	assert(t_begin + len < target.length);
	for(int i = 0; i < len; i++){
		target[t_begin + i] = data[d_begin + i];
	}
}

void readFloat32(Integer& target, Float32& data, int t_begin, int d_begin, int len){
	assert(t_begin >= 0 && d_begin >= 0);
	assert(t_begin + len < target.length);
	for(int i = 0; i < len; i++){
		data[t_begin + i] = target[d_begin + i];
	}
}


class District {

	// int d_id; //primary 
	// int d_w_id; //primary warehouse id
	// int d_next_o_id; //next available order number
	// float d_ytd; // year to date balance
	// float d_tax;
	// String d_name;
	// String d_street_1;
	// String d_street_2;
	// String d_city;
	// String d_state;
	// String d_zip;

public:
	int party_;
	Integer data_;

	District(int d_id, int d_w_id, int d_next_o_id, int p){
		party_ = p;
		Integer id = Integer(INT_LENGTH, d_id, party_);
		Integer w_id = Integer(INT_LENGTH, d_w_id, party_);
		Integer next_o_id = Integer(INT_LENGTH, d_next_o_id, party_);
		Float32 d_ytd = Float32(d_ytd, party_);
		Float32 d_tax = Float32(d_tax, party_);
		
		data_ = new Integer(INT_LENGTH * 3 + FLOAT32_LENGTH * 2, 0, party_);
		//TODO : use a combine function to simplify a long list of writeIntegers?
		writeInteger(data_, id, 0, 0, INT_LENGTH);
		writeInteger(data_, w_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, next_o_id, INT_LENGTH * 2, 0, INT_LENGTH);
		writeFloat32(data_, d_ytd, INT_LENGTH * 3, 0, FLOAT32_LENGTH);
		writeFloat32(data_, d_tax, INT_LENGTH * 3 + FLOAT32_LENGTH, 0, FLOAT32_LENGTH);
	}
}

class History {

	// int h_c_id;
	// int h_c_d_id;
	// int h_c_w_id;
	// int h_d_id;
	// int h_w_id;
	// Timestamp h_date;
	// float h_amount;
	// String h_data;
public:
	int party_;
	Integer data_;

	History(int cid, int cd_id, int cw_id, int did, int wid, int p){
		party_ = p;
		Integer c_id = Integer(INT_LENGTH, cid, party_);
		Integer c_d_id = Integer(INT_LENGTH, cd_id, party_);
		Integer c_w_id = Integer(INT_LENGTH, cw_id, party_);
		Integer d_id = Integer(INT_LENGTH, did, party_);
		Integer w_id = Integer(INT_LENGTH, wid, party_);

		data_ = new Integer(INT_LENGTH * 5, 0, party_);
		writeInteger(data_, c_id, 0, 0, INT_LENGTH);
		writeInteger(data_, c_d_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, c_w_id, INT_LENGTH * 2, 0, INT_LENGTH);
		writeInteger(data_, d_id, INT_LENGTH * 3, 0, INT_LENGTH);
		writeInteger(data_, w_id, INT_LENGTH * 4, 0, INT_LENGTH);

	}


}

struct NewOrderItem {
    // int32_t i_id;
    // int32_t ol_supply_w_id;
    // int32_t ol_quantity;
public:
	int party_;
	Integer data_;

	NewOrderItem(int i_id, int ol_supply_w_id, int ol_quantity, int p){
		party_ = p;
		Integer id = Integer(INT_LENGTH, i_id, party_);
		Integer supply_w_id = Integer(INT_LENGTH, ol_supply_w_id, party_);
		Integer quantity = Integer(INT_LENGTH, ol_quantity, party_);

		data_ = new Integer(INT_LENGTH * 3, 0, party_);
		writeInteger(data_, id, 0, 0, INT_LENGTH);
		writeInteger(data_, supply_w_id, 0, 0, INT_LENGTH);
		writeInteger(data_, quantity, 0, 0, INT_LENGTH);
	}

	Integer getItemID(){

	}

	Integer getSupplyWarehouseID(){

	}

	Integer getQuantity(){

	}

	

};

class Item {

	// int i_id; // PRIMARY KEY
	// int i_im_id;
    // double i_price;
	// String i_name;
	// String i_data;
public:
	int party_;
	Integer data_;

	Item(int id, int im_id, float price, int p){
		party_ = p;
		Integer id = Integer(INT_LENGTH, id, party_);
		Integer im_id = Integer(INT_LENGTH, im_id, party_);
		Float32 price = Float32(price, party_);

		data_ = new Integer(INT_LENGTH * 3, 0, party_);
		writeInteger(data_, id, 0, 0, INT_LENGTH);
		writeInteger(data_, im_id, INT_LENGTH, 0, INT_LENGTH);
		writeFloat32(data_, price, INT_LENGTH * 2, 0, FLOAT32_LENGTH);
	}

	Integer getItemID(){

	}

	Integer getItemImID(){

	}

	Float32 getPrice(){

	}


}

class NewOrder {

	// int no_w_id; //new order warehouse id
	// int no_d_id; //new order district id
	// int no_o_id; //new order id
public:
	int party_;
	Integer data_;

	NewOrder(int w_id, int d_id, int o_id, int p){
		party_ = p;
		Integer w_id = Integer(INT_LENGTH, w_id, party_);
		Integer d_id = Integer(INT_LENGTH, d_id, party_);
		Integer o_id = Integer(INT_LENGTH, o_id, party_);

		data_ = new Integer(INT_LENGTH * 3, 0, party_);
		writeInteger(data_, w_id, 0, 0, INT_LENGTH);
		writeInteger(data_, d_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, o_id, INT_LENGTH * 2, 0, INT_LENGTH);
	}

	Integer getWarehouseID(){

	}

	Integer getDistrictID(){

	}

	Integer getOrderID(){

	}
}

class Order {

	// int o_id;
	// int o_w_id;
	// int o_d_id;
	// int o_c_id;
	// Integer o_carrier_id;
	// int o_ol_cnt; //count of order lines
	// int o_all_local;
	// Timestamp o_entry_d;
public:
	int party_;
	Integer data_;

	Order(int o_id, int o_w_id, int o_d_id, int o_c_id, int o_ol_cnt, int p){
		party_ = p;
		Integer o_id = Integer(INT_LENGTH, o_id, party_);
		Integer o_w_id = Integer(INT_LENGTH, o_w_id, party_);
		Integer o_d_id = Integer(INT_LENGTH, o_d_id, party_);
		Integer o_c_id = Integer(INT_LENGTH, o_c_id, party_);
		Integer o_ol_cnt = Integer(INT_LENGTH, o_ol_cnt, party_);

		data_ = new Integer(INT_LENGTH * 5, 0, party_);
		writeInteger(data_, o_id, 0, 0, INT_LENGTH);
		writeInteger(data_, o_w_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, o_d_id, INT_LENGTH * 2, 0, INT_LENGTH);
		writeInteger(data_, o_c_id, INT_LENGTH * 3, 0, INT_LENGTH);
		writeInteger(data_, o_ol_cnt, INT_LENGTH * 4, 0, INT_LENGTH);
	}
}

class Stock {
public:

	// int s_i_id; // PRIMARY KEY 2
	// int s_w_id; // PRIMARY KEY 1 this is the item id
	// int s_order_cnt;
	// int s_remote_cnt;
	// int s_quantity;
	// float s_ytd;
	// String s_data;
	// String s_dist_01;
	// String s_dist_02;
	// String s_dist_03;
	// String s_dist_04;
	// String s_dist_05;
	// String s_dist_06;
	// String s_dist_07;
	// String s_dist_08;
	// String s_dist_09;
	// String s_dist_10;

	Integer data_;
	int party_;

	Stock(int s_i_id, int s_w_id, int s_order_cnt, int s_remote_cnt, int s_quantity, float s_ytd, int p){
		party_ = p;
		Integer s_i_id = Integer(INT_LENGTH, s_i_id, party_);
		Integer s_w_id = Integer(INT_LENGTH, s_w_id, party_);
		Integer s_order_cnt = Integer(INT_LENGTH, s_order_cnt, party_);
		Integer s_remote_cnt = Integer(INT_LENGTH, s_remote_cnt, party_);
		Integer s_quantity = Integer(INT_LENGTH, s_quantity, party_);
		Integer s_ytd = Float32(s_ytd, party_);

		data_ = new Integer(INT_LENGTH * 6, 0, party_);
		writeInteger(data_, s_i_id, 0, 0, INT_LENGTH);
		writeInteger(data_, s_w_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, s_order_cnt, INT_LENGTH * 2, 0, INT_LENGTH);
		writeInteger(data_, s_remote_cnt, INT_LENGTH * 3, 0, INT_LENGTH);
		writeInteger(data_, s_quantity, INT_LENGTH * 4, 0, INT_LENGTH);
		writeInteger(data_, s_ytd, INT_LENGTH * 5, 0, INT_LENGTH);
	}

	Integer getStockItemID(){

	}   

	Integer getStockWarehouseID(){

	}   

	Integer getStockOrderCnt(){

	}   

	Integer getStockRemoteCnt(){

	}   

	Integer getStockQuantity(){

	}        

	Integer getStockYTD(){

	}  

	void putStockQuantity(Integer q){

	}        

	void putStockOrderCnt(Integer cnt){

	}   

	void putStockYTD(Integer ytd){

	}                       

	void putStockRemoteCnt(Integer remote_cnt){

	}                                                                                                                                                                                                                                                                                                                                                                              

}

class Customer {

	// int c_id; //customer id, 3000 per district
	// int c_d_id; //district id, 20 unique ids
	// int c_w_id; //warehouse id, 2 * W unique ids
	//above three are primary key

	// int c_payment_cnt;
	// int c_delivery_cnt;
	// Timestamp c_since;
	// float c_discount;
	// float c_credit_lim;
	// float c_balance;
	// float c_ytd_payment;
	// String c_credit;
	// String c_last;
	// String c_first;
	// String c_street_1;
	// String c_street_2;
	// String c_city;
	// String c_state;
	// String c_zip;
	// String c_phone;
	// String c_middle;
	// String c_data;

public:
	Integer data_;
	int party_;

	Customer(int c_id, int c_d_id, int c_w_id, int c_payment_cnt, int c_delivery_cnt, 
			float c_discount, float c_credit_lim, float c_balance, float c_ytd_payment, int p){
				party_ = p;
				Integer c_id = Integer(INT_LENGTH, c_id, party_);
				Integer c_d_id = Integer(INT_LENGTH, c_d_id, party_);
				Integer c_w_id = Integer(INT_LENGTH, c_w_id, party_);
				Integer c_payment_cnt = Integer(INT_LENGTH, c_payment_cnt, party_);
				Integer c_delivery_cnt = Integer(INT_LENGTH, c_delivery_cnt, party_);
				Float32 c_discount = Float32(c_discount, party_);
				Float32 c_credit_lim = Float32(c_credit_lim, party_);
				Float32 c_balance = Float32(c_balance, party_);
				Float32 c_ytd_payment = Float32(c_ytd_payment, party_);

				data_ = new Integer(INT_LENGTH * 5 + FLOAT32_LENGTH * 4, 0, party_);
				writeInteger(data_, c_id, 0, 0, INT_LENGTH);
				writeInteger(data_, c_d_id, INT_LENGTH, 0, INT_LENGTH);
				writeInteger(data_, c_w_id, INT_LENGTH * 2, 0, INT_LENGTH);
				writeInteger(data_, c_payment_cnt, INT_LENGTH * 3, 0, INT_LENGTH);
				writeInteger(data_, c_delivery_cnt, INT_LENGTH * 4, 0, INT_LENGTH);

				writeFloat32(data_, c_discount, INT_LENGTH * 5, 0, FLOAT32_LENGTH);
				writeFloat32(data_, c_credit_lim, INT_LENGTH * 5 + FLOAT32_LENGTH, 0, FLOAT32_LENGTH);
				writeFloat32(data_, c_balance, INT_LENGTH * 5 + FLOAT32_LENGTH * 2, 0, FLOAT32_LENGTH);
				writeFloat32(data_, c_ytd_payment, INT_LENGTH * 5 + FLOAT32_LENGTH * 3, 0, FLOAT32_LENGTH);
			}

}

class Warehouse {

	// int w_id; // PRIMARY KEY warehouse id
	// float w_ytd; // year to dat balance
    // double w_tax;
	// String w_name;
	// String w_street_1;
	// String w_street_2;
	// String w_city;
	// String w_state;
	// String w_zip;

public:
	Integer data_;
	int party_;

	Warehouse(int w_id, int w_ytd, float w_tax, int p){
		party_ = p;
		Integer w_id = Integer(INT_LENGTH, w_id, party_);
		Integer w_ytd = Integer(INT_LENGTH, w_ytd, party_);
		Float32j w_tax = Float32(w_tax, party_);
	}
}

class OrderLine {

	// int ol_w_id;
	// int ol_d_id;
	// int ol_o_id;
	// int ol_number;
	// int ol_i_id;
	// int ol_supply_w_id;
	// int ol_quantity;
	// Timestamp ol_delivery_d;
	// float ol_amount;
	// String ol_dist_info;

public:
	Integer data_;
	int party_;

	OrderLine(int ol_w_id, int ol_d_id, int ol_o_id, int ol_number, int ol_i_id, int ol_supply_w_id, int ol_quantity, float ol_amount, int p){
		party_ = p;
		Integer ol_w_id = Integer(INT_LENGTH, ol_w_id, party_);
		Integer ol_d_id = Integer(INT_LENGTH, ol_d_id, party_);
		Integer ol_o_id = Integer(INT_LENGTH, ol_o_id, party_);
		Integer ol_i_id = Integer(INT_LENGTH, ol_i_id, party_);
		Integer ol_number = Integer(INT_LENGTH, ol_i_id, party_);
		Integer ol_supply_w_id = Integer(INT_LENGTH, ol_supply_w_id, party_);
		Integer ol_quantity = Integer(INT_LENGTH, ol_quantity, party_);
		Float32 ol_amount = Float32(ol_amount, party_);

		data_ = new Integer(INT_LENGTH * 7 + FLOAT32_LENGTH, 0, party_);

		writeInteger(data_, ol_w_id, 0, 0, INT_LENGTH);
		writeInteger(data_, ol_d_id, INT_LENGTH, 0, INT_LENGTH);
		writeInteger(data_, ol_o_id, INT_LENGTH * 2, 0, INT_LENGTH);
		writeInteger(data_, ol_i_id, INT_LENGTH * 3, 0, INT_LENGTH);
		writeInteger(data_, ol_number, INT_LENGTH * 4, 0, INT_LENGTH);
		writeInteger(data_, ol_supply_w_id, INT_LENGTH * 5, 0, INT_LENGTH);
		writeInteger(data_, ol_quantity, INT_LENGTH * 6, 0, INT_LENGTH);
		writeFloat32(data_, ol_amount, INT_LENGTH * 7, 0, FLOAT32_LENGTH);

	}

	Integer getOrderlineOrderID(){

	}

	Integer getOrderlineDistrictID(){

	}

	Integer getOrderlineWarehouseID(){

	}

	Integer getOrderlineItemID(){

	}

	void putOrderlineOrderID(Integer o_id){

	}

	void putOrderlineDistrictID(Integer d_id){

	}

	void putOrderlineWarehouseID(Integer w_id){

	}
}

// Contains data required to undo transactions. Note that only new order, payment, and delivery
// update the database. This structure only contains information to undo these transactions.
class TPCCUndo {
public:
    ~TPCCUndo();

    void save(Warehouse* w);
    void save(District* d);
    void save(Customer* c);
    void save(Stock* s);
    void save(Order* o);                                 
    void save(OrderLine* o);

    void inserted(const Order* o);
    void inserted(const OrderLine* ol);
    void inserted(const NewOrder* no);                                                                 
    void inserted(const History* h);

    void deleted(NewOrder* no);

    // Marks this undo buffer as applied. This prevents the destructor from deleting tuples
    // marked as deleted.
    void applied();

    typedef std::unordered_map<Warehouse*, Warehouse*> WarehouseMap;
    const WarehouseMap& modified_warehouses() const { return modified_warehouses_; }

    typedef std::unordered_map<District*, District*> DistrictMap;
    const DistrictMap& modified_districts() const { return modified_districts_; }

    typedef std::unordered_map<Customer*, Customer*> CustomerMap;
    const CustomerMap& modified_customers() const { return modified_customers_; }

    typedef std::unordered_map<Stock*, Stock*> StockMap;
    const StockMap& modified_stock() const { return modified_stock_; }

    typedef std::unordered_map<Order*, Order*> OrderMap;
    const OrderMap& modified_orders() const { return modified_orders_; }

    typedef std::unordered_map<OrderLine*, OrderLine*> OrderLineMap;
    const OrderLineMap& modified_order_lines() const { return modified_order_lines_; }

    typedef tpcc::Set<const Order*> OrderSet;
    const OrderSet& inserted_orders() const { return inserted_orders_; }

    typedef tpcc::Set<const OrderLine*> OrderLineSet;
    const OrderLineSet& inserted_order_lines() const { return inserted_order_lines_; }

    typedef tpcc::Set<const NewOrder*> NewOrderSet;
    const NewOrderSet& inserted_new_orders() const { return inserted_new_orders_; }
    typedef tpcc::Set<NewOrder*> NewOrderDeletedSet;
    const NewOrderDeletedSet& deleted_new_orders() const { return deleted_new_orders_; }

    typedef tpcc::Set<const History*> HistorySet;
    const HistorySet& inserted_history() const { return inserted_history_; }

private:
    WarehouseMap modified_warehouses_;
    DistrictMap modified_districts_;
    CustomerMap modified_customers_;
    StockMap modified_stock_;
    OrderMap modified_orders_;
    OrderLineMap modified_order_lines_;

    OrderSet inserted_orders_;
    OrderLineSet inserted_order_lines_;
    NewOrderSet inserted_new_orders_;
    HistorySet inserted_history_;

    NewOrderDeletedSet deleted_new_orders_;
};


class TPCCDB{
public:
    //B+ tree should be used instead of vector

	int party_;
    std::vector<Item*> items_;
    std::vector<History*> history_;
    std::vector<Warehouse*> warehouses_;
    std::vector<Stock*> stocks_;
    std::vector<District*> districts_;
    std::vector<Customer*> customers_;
    std::vector<Order*> orders_;
    std::vector<OrderLine*> orderlines_;
    std::vector<NewOrder*> neworders_;



    TPCCDB();
    ~TPCCDB();

	void loadItems(string filename);
	void loadHistroy(string filename);
	void loadWarehouses(string filename);
	void loadStocks(string filename);
	void loadDistricts(string filename);
	void loadCustomers(string filename);
	void loadOrders(string filename);
	void loadOrderLines(string filename);
	void loadNewOrders(string filename);


	virtual bool newOrder(int32_t warehouse_id, int32_t district_id, int32_t customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    virtual bool newOrderHome(int32_t warehouse_id, int32_t district_id, int32_t customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    virtual bool newOrderRemote(int32_t home_warehouse, int32_t remote_warehouse,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
	typedef tpcc::Set<int32_t> WarehouseSet;
    static WarehouseSet newOrderRemoteWarehouses(int32_t home_warehouse, const std::vector<NewOrderItem>& items);

    static const int32_t INVALID_QUANTITY = -1;
    // Combines valid quantities into output.

	// Copies item into the item table.
    void insertItem(const Item& item);
    Item* findItem(int32_t i_id);

    void insertWarehouse(const Warehouse& warehouse);
    Warehouse* findWarehouse(int32_t w_id);

    void insertStock(const Stock& stock);
    Stock* findStock(int32_t w_id, int32_t s_id);

    void insertDistrict(const District& district);
    District* findDistrict(int32_t w_id, int32_t d_id);

    void insertCustomer(const Customer& customer);
    Customer* findCustomer(int32_t w_id, int32_t d_id, int32_t c_id);
    // Finds all customers that match (w_id, d_id, *, c_last), taking the n/2th one (rounded up).

    // Stores order in the database. Returns a pointer to the database's tuple.
    Order* insertOrder(const Order& order);
    Order* findOrder(int32_t w_id, int32_t d_id, int32_t o_id);

    // Stores orderline in the database. Returns a pointer to the database's tuple.
    OrderLine* insertOrderLine(const OrderLine& orderline);
    OrderLine* findOrderLine(int32_t w_id, int32_t d_id, int32_t o_id, int32_t number);

    // Creates a new order in the database. Returns a pointer to the database's tuple.
    NewOrder* insertNewOrder(int32_t w_id, int32_t d_id, int32_t o_id);
    NewOrder* findNewOrder(int32_t w_id, int32_t d_id, int32_t o_id);

	// Stores order in the database. Returns a pointer to the database's tuple.
    History* insertHistory(const History& history);

private:

	// Erases order from the database. NOTE: This is only for undoing transactions.
    void eraseOrder(const Order* order);
    // Erases order_line from the database. NOTE: This is only for undoing transactions.
    void eraseOrderLine(const OrderLine* order_line);
    // Erases new_order from the database. NOTE: This is only for undoing transactions.
    void eraseNewOrder(const NewOrder* new_order);
    // Erases history from the database. NOTE: This is only for undoing transactions.
    void eraseHistory(const History* history);

}