#pragma once
#include <typeinfo>
#include <emp-tool/emp-tool.h>
#include <emp-sh2pc/emp-sh2pc.h>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include<iostream>
#include<string>
#include<fstream>
#include<algorithm>
#include<sstream>

using namespace std;
using namespace emp;

#define INT_LENGTH 32


vector<std::string> split(std::string str, char delimiter){
    vector<std::string> internal;
    std::stringstream ss(str); // Turn the string into a stream.
    string tok;
    
    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }
    
    return internal;  
}
// void writeInteger(Integer& target, Integer& data, int t_begin, int d_begin, int len){
// 	ASSERT(t_begin + len < target.length);
// 	ASSERT(d_begin + len < data.length);
// 	for(int i = 0; i < len; i++){
// 		target[t_begin + i] = data[d_begin + i]; // copy data Bits to override target Bits
// 	}
// }

// void readInteger(Integer& target, Integer& data, int t_begin, int d_begin, int len){
// 	ASSERT(t_begin >= 0 && d_begin >= 0);
// 	ASSERT(t_begin + len < target.length);
// 	ASSERT(d_begin + len < data.length);
// 	for(int i = 0; i < len; i++){
// 		data[t_begin + i] = target[d_begin + i]; 
// 	}
// }

// void writeFloat32(Integer& target, Float32& data, int t_begin, int d_begin, int len){
// 	ASSERT(t_begin >= 0 && d_begin >= 0);
// 	ASSERT(t_begin + len < target.length);
// 	for(int i = 0; i < len; i++){
// 		target[t_begin + i] = data[d_begin + i];
// 	}
// }

// void readFloat32(Integer& target, Float32& data, int t_begin, int d_begin, int len){
// 	ASSERT(t_begin >= 0 && d_begin >= 0);
// 	ASSERT(t_begin + len < target.length);
// 	for(int i = 0; i < len; i++){
// 		data[t_begin + i] = target[d_begin + i];
// 	}
// }


class District {
public:
	Integer d_id; //primary 
	Integer d_w_id; //primary warehouse id
	Integer d_next_o_id; //next available order number
	Integer d_ytd; // year to date balance
	Integer d_tax;
	int party_;

	static const int NUM_PER_WAREHOUSE = 10;
	// String d_name;
	// String d_street_1;
	// String d_street_2;
	// String d_city;
	// String d_state;
	// String d_zip;

	District(int p){
		party_ = p;
	}

	District(Integer id, Integer w_id, Integer next_o_id, Integer ytd, Integer tax, int p){
		party_ = p;
		d_id = id;
		d_w_id = w_id;
		d_next_o_id = next_o_id;
		d_ytd = ytd;
		d_tax = tax;
	}

};

class History {
public:
	int party_;
	Integer h_c_id;
	Integer h_c_d_id;
	Integer h_c_w_id;
	Integer h_d_id;
	Integer h_w_id;
	// Timestamp h_date;
	Integer h_amount;
	// String h_data;

	History(int p){
		party_ = p;
	}


};

struct NewOrderItem {
public:
	int party_;
    Integer ol_i_id;
    Integer ol_supply_w_id;
    Integer ol_quantity;

	NewOrderItem(int p){
		party_ = p;
	}

	NewOrderItem(Integer id, Integer supply_w_id, Integer quantity, int p){
		party_ = p;
		ol_i_id = id;
		ol_supply_w_id = supply_w_id;
		ol_quantity = quantity;
	}


};

class Item {
public:
	int party_;
	Integer i_id; // PRIMARY KEY
	Integer i_im_id;
    Integer i_price;
	// String i_name;
	// String i_data;


	Item(int p){
		party_ = p;
	}

	Item(Integer id, Integer im_id, Integer price, int p){
		party_ = p;
		i_id = id;
		i_im_id = im_id;
		i_price = price;
	}


};

class NewOrder {
public:
	int party_;
	Integer no_w_id; //new order warehouse id
	Integer no_d_id; //new order district id
	Integer no_o_id; //new order id


	NewOrder(int p){
		party_ = p;
	}

	NewOrder(Integer w_id, Integer d_id, Integer o_id, int p){
		no_w_id = w_id;
		no_d_id = d_id;
		no_o_id = o_id;
		party_ = p;
	}

};

class Order {
public:
	int party_;
	Integer o_id;
	Integer o_w_id;
	Integer o_d_id;
	Integer o_c_id;
	Integer o_ol_cnt; //count of order lines
	// int o_all_local;
	// Timestamp o_entry_d;
	// Integer o_carrier_id;

	Order(int p){
		party_ = p;
	}

	Order(Integer id, Integer w_id, Integer d_id, Integer c_id, Integer ol_cnt, int p){
		party_ = p;
		o_id = id;
		o_w_id = w_id;
		o_d_id = d_id;
		o_c_id = c_id;
		o_ol_cnt = ol_cnt;
	}
};

class Stock {
public:
	int party_;
	Integer s_i_id; // PRIMARY KEY 2
	Integer s_w_id; // PRIMARY KEY 1 this is the item id
	Integer s_order_cnt;
	Integer s_remote_cnt;
	Integer s_quantity;
	Integer s_ytd;
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

	Stock(int p){
		party_ = p;
	}

	Stock(Integer i_id, Integer w_id, Integer order_cnt, Integer remote_cnt, Integer quantity, Integer ytd, int p){
		party_ = p;
		s_i_id = i_id;
		s_w_id = w_id;
		s_order_cnt = order_cnt;
		s_remote_cnt = remote_cnt;
		s_quantity = quantity;
		s_ytd = ytd;
	}


};

class Customer {
public:
	int party_;
	Integer c_id; //customer id, 3000 per district
	Integer c_d_id; //district id, 20 unique ids
	Integer c_w_id; //warehouse id, 2 * W unique ids
	//above three are primary key

	Integer c_payment_cnt;
	Integer c_delivery_cnt;
	// Timestamp c_since;
	Integer c_discount;
	//float c_credit_lim;
	Integer c_balance;
	Integer c_ytd_payment;
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


	Customer(int p){
		party_ = p;
	}

	Customer(Integer id, Integer d_id, Integer w_id, Integer payment_cnt, Integer delivery_cnt, Integer discount,
	 		 Integer balance, Integer ytd_payment, int p){
				party_  = p;
				c_id = id;
				c_d_id = d_id;
				c_w_id = w_id;
				c_payment_cnt = payment_cnt;
				c_delivery_cnt = delivery_cnt;
				c_discount = discount;
				c_balance = balance;
				c_ytd_payment = ytd_payment;
		}


};

class Warehouse {
public:
	int party_;
	Integer w_id; // PRIMARY KEY warehouse id
	Integer w_ytd; // year to dat balance
    Integer w_tax;
	// String w_name;
	// String w_street_1;
	// String w_street_2;
	// String w_city;
	// String w_state;
	// String w_zip;

	Warehouse(int p){
		party_ = p;
	}

	Warehouse(Integer id, Integer ytd, Integer tax, int p){
		party_ = p;
		w_id = id;
		w_ytd = ytd;
		w_tax = tax;
	}

};

class OrderLine {
public:
	int party_;
	Integer ol_w_id;
	Integer ol_d_id;
	Integer ol_o_id;
	Integer ol_number;
	Integer ol_i_id;
	Integer ol_supply_w_id;
	Integer ol_quantity;
	// Timestamp ol_delivery_d;
	Integer ol_amount;
	// String ol_dist_info;

	OrderLine(int p){
		party_ = p;
	}

	OrderLine(Integer w_id, Integer d_id, Integer o_id, Integer number, Integer i_id, Integer supply_w_id, Integer quantity, Integer amount, int p){
		party_ = p;
		ol_w_id = w_id;
		ol_d_id = d_id;
		ol_o_id = o_id;
		ol_number = number;
		ol_i_id = i_id;
		ol_supply_w_id = supply_w_id;
		ol_quantity = quantity;
		ol_amount = amount;
	}

};

struct DeliveryOrderInfo {
    Integer d_id;
    Integer o_id;
};

// Contains data required to undo transactions. Note that only new order, payment, and delivery
// update the database. This structure only contains information to undo these transactions.
class TPCCUndo {
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
};


class TPCCDB{
public:
    //B+ tree should be used instead of vector

	int party_;
	int port_;
	std::string aliceHost = "127.0.0.1";
    std::vector<Item*> items_;
    std::vector<History*> history_;
    std::vector<Warehouse*> warehouses_;
    std::vector<Stock*> stocks_;
    std::vector<District*> districts_;
    std::vector<Customer*> customers_;
    std::vector<Order*> orders_;
    std::vector<OrderLine*> orderlines_;
    std::vector<NewOrder*> neworders_;



    TPCCDB(int party, int port);
    ~TPCCDB();

	
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

	void delivery(Integer warehouse_id, Integer carrier_id, 
        std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo);


	void payment(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer customer_id, Integer h_amount, TPCCUndo** undo);

    void paymentHome(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo);
    void paymentRemote(Integer warehouse_id, Integer district_id, Integer c_warehouse_id,
            Integer c_district_id, Integer c_id, Integer h_amount, TPCCUndo** undo);

	void internalPaymentRemote(Integer warehouse_id, Integer district_id, Customer* c,
        Integer h_amount,  TPCCUndo** undo);



	bool findAndValidateItems(const vector<NewOrderItem>& items, vector<Item*>* item_tuples);

    Item* findItem(Integer i_id);

    Warehouse* findWarehouse(Integer w_id);

    Stock* findStock(Integer w_id, Integer s_id);

    District* findDistrict(Integer w_id, Integer d_id);

    Customer* findCustomer(Integer w_id, Integer d_id, Integer c_id);

    // Stores order in the database. Returns a pointer to the database's tuple.
    Order* findOrder(Integer w_id, Integer d_id, Integer o_id);

    // Stores orderline in the database. Returns a pointer to the database's tuple.
    OrderLine* findOrderLine(Integer w_id, Integer d_id, Integer o_id, Integer number);

    // Creates a new order in the database. Returns a pointer to the database's tuple.
    NewOrder* findNewOrder(Integer w_id, Integer d_id, Integer o_id);


private:

	// Erases order from the database. NOTE: This is only for undoing transactions.
    void eraseOrder(const Order* order);
    // Erases order_line from the database. NOTE: This is only for undoing transactions.
    void eraseOrderLine(const OrderLine* order_line);
    // Erases new_order from the database. NOTE: This is only for undoing transactions.
    void eraseNewOrder(const NewOrder* new_order);
    // Erases history from the database. NOTE: This is only for undoing transactions.
    void eraseHistory(const History* history);

};