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

	District(int id, int w_id, int next_o_id, int ytd, int tax, int p){
		party_ = p;
		d_id = Integer(INT_LENGTH, id, party_);
		d_w_id = Integer(INT_LENGTH, w_id, party_);
		d_next_o_id = Integer(INT_LENGTH, next_o_id, party_);
		d_ytd = Integer(INT_LENGTH, ytd, party_);
		d_tax = Integer(INT_LENGTH, tax, party_);
	}

	District(Integer id, Integer w_id, Integer next_o_id, Integer ytd, Integer tax, int p){
		party_ = p;
		d_id = id
		d_w_id = w_id
		d_next_o_id = next_o_id;
		d_ytd = ytd;
		d_tax = tax;
	}


	// District(int d_id, int d_w_id, int d_next_o_id, int d_ytd, int d_tax, int p){
	// 	party_ = p;
	// 	Integer id = Integer(INT_LENGTH, d_id, party_);
	// 	Integer w_id = Integer(INT_LENGTH, d_w_id, party_);
	// 	Integer next_o_id = Integer(INT_LENGTH, d_next_o_id, party_);
	// 	Float32 d_ytd = Float32(d_ytd, party_);
	// 	Float32 d_tax = Float32(d_tax, party_);
		
	// 	data_ = new Integer(INT_LENGTH * 3 + FLOAT32_LENGTH * 2, 0, party_);
	// 	writeInteger(data_, id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, w_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, next_o_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// 	writeFloat32(data_, d_ytd, INT_LENGTH * 3, 0, FLOAT32_LENGTH);
	// 	writeFloat32(data_, d_tax, INT_LENGTH * 3 + FLOAT32_LENGTH, 0, FLOAT32_LENGTH);
	// }
}

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


	// History(int cid, int cd_id, int cw_id, int did, int wid, int p){
	// 	party_ = p;
	// 	Integer c_id = Integer(INT_LENGTH, cid, party_);
	// 	Integer c_d_id = Integer(INT_LENGTH, cd_id, party_);
	// 	Integer c_w_id = Integer(INT_LENGTH, cw_id, party_);
	// 	Integer d_id = Integer(INT_LENGTH, did, party_);
	// 	Integer w_id = Integer(INT_LENGTH, wid, party_);

	// 	data_ = new Integer(INT_LENGTH * 5, 0, party_);
	// 	writeInteger(data_, c_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, c_d_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, c_w_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// 	writeInteger(data_, d_id, INT_LENGTH * 3, 0, INT_LENGTH);
	// 	writeInteger(data_, w_id, INT_LENGTH * 4, 0, INT_LENGTH);

	// }


}

struct NewOrderItem {
public:
	int party_;
    Integer i_id;
    Integer ol_supply_w_id;
    Integer ol_quantity;

	NewOrderItem(int p){
		party_ = p;
	}

	NewOrderItem(Integer id, Integer supply_w_id, Integer quantity, int p){
		party_ = p;
		i_id = id;
		ol_supply_w_id = supply_w_id;
		ol_quantity = quantity;
	}

	NewOrderItem(int id, int supply_w_id, int quantity, int p){
		party_ = p;
		i_id = Integer(INT_LENGTH, id, party_);
		i_supply_w_id = Integer(INT_LENGTH, supply_w_id, party_);
		i_quantity = Integer(INT_LENGTH, quantity, party_);
	}

	// NewOrderItem(int i_id, int ol_supply_w_id, int ol_quantity, int p){
	// 	party_ = p;
	// 	Integer id = Integer(INT_LENGTH, i_id, party_);
	// 	Integer supply_w_id = Integer(INT_LENGTH, ol_supply_w_id, party_);
	// 	Integer quantity = Integer(INT_LENGTH, ol_quantity, party_);

	// 	data_ = new Integer(INT_LENGTH * 3, 0, party_);
	// 	writeInteger(data_, id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, supply_w_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, quantity, 0, 0, INT_LENGTH);
	// }


};

class Item {
public:
	int party_;
	Integer i_id; // PRIMARY KEY
	Integer i_im_id;
    Integer i_price;
	// String i_name;
	// String i_data;

	Item(int id, int im_id, float price, int p){
		party_ = p;
		i_id = Integer(INT_LENGTH, id, party_);
		i_im_id = Integer(INT_LENGTH, im_id, party_);
		i_price = Integer(INT_LENGTH, price, party_);
	}

	Item(int p){
		party_ = p;
	}

	Item(Integer id, Integer im_id, Integer price, int p){
		party_ = p;
		i_id = id;
		i_im_id = im_id;
		i_price = price;
	}

	// Item(int id, int im_id, float price, int p){
	// 	party_ = p;
	// 	Integer id = Integer(INT_LENGTH, id, party_);
	// 	Integer im_id = Integer(INT_LENGTH, im_id, party_);
	// 	Float32 price = Float32(price, party_);

	// 	data_ = new Integer(INT_LENGTH * 3, 0, party_);
	// 	writeInteger(data_, id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, im_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeFloat32(data_, price, INT_LENGTH * 2, 0, FLOAT32_LENGTH);
	// }


}

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
	NewOrder(int w_id, int d_id, int o_id, int p){
		party_ = p;
		no_w_id = Integer(INT_LENGTH, w_id, party_);
		no_d_id = Integer(INT_LENGTH, d_id, party_);
		no_o_id = Integer(INT_LENGTH, o_id, party_);

	}

	// NewOrder(int w_id, int d_id, int o_id, int p){
	// 	party_ = p;
	// 	Integer w_id = Integer(INT_LENGTH, w_id, party_);
	// 	Integer d_id = Integer(INT_LENGTH, d_id, party_);
	// 	Integer o_id = Integer(INT_LENGTH, o_id, party_);

	// 	data_ = new Integer(INT_LENGTH * 3, 0, party_);
	// 	writeInteger(data_, w_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, d_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, o_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// }

}

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
	Order(int id, int w_id, int d_id, int c_id, int ol_cnt, int p){
		party_ = p;
		o_id = Integer(INT_LENGTH, id, party_);
		o_w_id = Integer(INT_LENGTH, w_id, party_);
		o_d_id = Integer(INT_LENGTH, d_id, party_);
		o_c_id = Integer(INT_LENGTH, c_id, party_);
		o_ol_cnt = Integer(INT_LENGTH, ol_cnt, party_);
	}

	// Order(int o_id, int o_w_id, int o_d_id, int o_c_id, int o_ol_cnt, int p){
	// 	party_ = p;
	// 	Integer o_id = Integer(INT_LENGTH, o_id, party_);
	// 	Integer o_w_id = Integer(INT_LENGTH, o_w_id, party_);
	// 	Integer o_d_id = Integer(INT_LENGTH, o_d_id, party_);
	// 	Integer o_c_id = Integer(INT_LENGTH, o_c_id, party_);
	// 	Integer o_ol_cnt = Integer(INT_LENGTH, o_ol_cnt, party_);

	// 	data_ = new Integer(INT_LENGTH * 5, 0, party_);
	// 	writeInteger(data_, o_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, o_w_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, o_d_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// 	writeInteger(data_, o_c_id, INT_LENGTH * 3, 0, INT_LENGTH);
	// 	writeInteger(data_, o_ol_cnt, INT_LENGTH * 4, 0, INT_LENGTH);
	// }
}

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

	Stock(int i_id, int w_id, int order_cnt, int remote_cnt, int quantity, int ytd, int p){
		party_ = p;
		s_i_id = Integer(INT_LENGTH, i_id, party_);
		s_w_id = Integer(INT_LENGTH, w_id, party_);
		s_order_cnt = Integer(INT_LENGTH, order_cnt, party_);
		s_remote_cnt = Integer(INT_LENGTH, remote_cnt, party_);
		s_quantity = Integer(INT_LENGTH, quantity, party_);
		s_ytd = Integer(INT_LENGTH, ytd, party_);
	}

	// Stock(int s_i_id, int s_w_id, int s_order_cnt, int s_remote_cnt, int s_quantity, float s_ytd, int p){
	// 	party_ = p;
	// 	Integer s_i_id = Integer(INT_LENGTH, s_i_id, party_);
	// 	Integer s_w_id = Integer(INT_LENGTH, s_w_id, party_);
	// 	Integer s_order_cnt = Integer(INT_LENGTH, s_order_cnt, party_);
	// 	Integer s_remote_cnt = Integer(INT_LENGTH, s_remote_cnt, party_);
	// 	Integer s_quantity = Integer(INT_LENGTH, s_quantity, party_);
	// 	Integer s_ytd = Float32(s_ytd, party_);

	// 	data_ = new Integer(INT_LENGTH * 6, 0, party_);
	// 	writeInteger(data_, s_i_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, s_w_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, s_order_cnt, INT_LENGTH * 2, 0, INT_LENGTH);
	// 	writeInteger(data_, s_remote_cnt, INT_LENGTH * 3, 0, INT_LENGTH);
	// 	writeInteger(data_, s_quantity, INT_LENGTH * 4, 0, INT_LENGTH);
	// 	writeInteger(data_, s_ytd, INT_LENGTH * 5, 0, INT_LENGTH);
	// }

}

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

	Customer(int id, int d_id, int w_id, int payment_cnt, int delivery_cnt, 
			int discount, int balance, int ytd_payment, int p){
				party_ = p;
				c_id = Integer(INT_LENGTH, id, party_);
				c_d_id = Integer(INT_LENGTH, d_id, party_);
				c_w_id = Integer(INT_LENGTH, w_id, party_);
				c_payment_cnt = Integer(INT_LENGTH, payment_cnt, party_);
				c_delivery_cnt = Integer(INT_LENGTH, delivery_cnt, party_);
				c_discount = Integer(INT_LENGTH, discount, party_);
				c_balance = Integer(INT_LENGTH, balance, party_);
				c_ytd_payment = Integer(INT_LENGTH, ytd_payment, party_);
		}

	// Customer(int c_id, int c_d_id, int c_w_id, int c_payment_cnt, int c_delivery_cnt, 
	// 		float c_discount, float c_credit_lim, float c_balance, float c_ytd_payment, int p){
	// 			party_ = p;
	// 			Integer c_id = Integer(INT_LENGTH, c_id, party_);
	// 			Integer c_d_id = Integer(INT_LENGTH, c_d_id, party_);
	// 			Integer c_w_id = Integer(INT_LENGTH, c_w_id, party_);
	// 			Integer c_payment_cnt = Integer(INT_LENGTH, c_payment_cnt, party_);
	// 			Integer c_delivery_cnt = Integer(INT_LENGTH, c_delivery_cnt, party_);
	// 			Float32 c_discount = Float32(c_discount, party_);
	// 			Float32 c_credit_lim = Float32(c_credit_lim, party_);
	// 			Float32 c_balance = Float32(c_balance, party_);
	// 			Float32 c_ytd_payment = Float32(c_ytd_payment, party_);

	// 			data_ = new Integer(INT_LENGTH * 5 + FLOAT32_LENGTH * 4, 0, party_);
	// 			writeInteger(data_, c_id, 0, 0, INT_LENGTH);
	// 			writeInteger(data_, c_d_id, INT_LENGTH, 0, INT_LENGTH);
	// 			writeInteger(data_, c_w_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// 			writeInteger(data_, c_payment_cnt, INT_LENGTH * 3, 0, INT_LENGTH);
	// 			writeInteger(data_, c_delivery_cnt, INT_LENGTH * 4, 0, INT_LENGTH);

	// 			writeFloat32(data_, c_discount, INT_LENGTH * 5, 0, FLOAT32_LENGTH);
	// 			writeFloat32(data_, c_credit_lim, INT_LENGTH * 5 + FLOAT32_LENGTH, 0, FLOAT32_LENGTH);
	// 			writeFloat32(data_, c_balance, INT_LENGTH * 5 + FLOAT32_LENGTH * 2, 0, FLOAT32_LENGTH);
	// 			writeFloat32(data_, c_ytd_payment, INT_LENGTH * 5 + FLOAT32_LENGTH * 3, 0, FLOAT32_LENGTH);
	// 		}

}

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

	Warehouse(int id, int ytd, float tax, int p){
		party_ = p;
		w_id = Integer(INT_LENGTH, id, party_);
		w_ytd = Integer(INT_LENGTH, ytd, party_);
		w_tax = Integer(INT_LENGTH, tax, party_);
	}


	// Warehouse(int w_id, int w_ytd, float w_tax, int p){
	// 	party_ = p;
	// 	Integer w_id = Integer(INT_LENGTH, w_id, party_);
	// 	Integer w_ytd = Integer(INT_LENGTH, w_ytd, party_);
	// 	Float32j w_tax = Float32(w_tax, party_);
	// }
}

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

	OrderLine(int ol_w_id, int ol_d_id, int ol_o_id, int ol_number, int ol_i_id, int ol_supply_w_id, int ol_quantity, int ol_amount, int p){
		party_ = p;
		ol_w_id = Integer(INT_LENGTH, ol_w_id, party_);
		ol_d_id = Integer(INT_LENGTH, ol_d_id, party_);
		ol_o_id = Integer(INT_LENGTH, ol_o_id, party_);
		ol_number = Integer(INT_LENGTH, ol_number, party_);
		ol_i_id = Integer(INT_LENGTH, ol_i_id, party_);
		ol_supply_w_id = Integer(INT_LENGTH, ol_supply_w_id, party_);
		ol_quantity = Integer(INT_LENGTH, ol_quantity, party_);
		ol_amount = Integer(INT_LENGTH, ol_amount, party_);
	}

	// OrderLine(int ol_w_id, int ol_d_id, int ol_o_id, int ol_number, int ol_i_id, int ol_supply_w_id, int ol_quantity, float ol_amount, int p){
	// 	party_ = p;
	// 	Integer ol_w_id = Integer(INT_LENGTH, ol_w_id, party_);
	// 	Integer ol_d_id = Integer(INT_LENGTH, ol_d_id, party_);
	// 	Integer ol_o_id = Integer(INT_LENGTH, ol_o_id, party_);
	// 	Integer ol_i_id = Integer(INT_LENGTH, ol_i_id, party_);
	// 	Integer ol_number = Integer(INT_LENGTH, ol_i_id, party_);
	// 	Integer ol_supply_w_id = Integer(INT_LENGTH, ol_supply_w_id, party_);
	// 	Integer ol_quantity = Integer(INT_LENGTH, ol_quantity, party_);
	// 	Float32 ol_amount = Float32(ol_amount, party_);

	// 	data_ = new Integer(INT_LENGTH * 7 + FLOAT32_LENGTH, 0, party_);

	// 	writeInteger(data_, ol_w_id, 0, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_d_id, INT_LENGTH, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_o_id, INT_LENGTH * 2, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_i_id, INT_LENGTH * 3, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_number, INT_LENGTH * 4, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_supply_w_id, INT_LENGTH * 5, 0, INT_LENGTH);
	// 	writeInteger(data_, ol_quantity, INT_LENGTH * 6, 0, INT_LENGTH);
	// 	writeFloat32(data_, ol_amount, INT_LENGTH * 7, 0, FLOAT32_LENGTH);

	// }

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


	virtual bool newOrder(Integer warehouse_id, Integer district_id, Integer customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    virtual bool newOrderHome(Integer warehouse_id, Integer district_id, Integer customer_id,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
    virtual bool newOrderRemote(Integer home_warehouse, Integer remote_warehouse,
            const std::vector<NewOrderItem>& items, TPCCUndo** undo);
	typedef tpcc::Set<Integer> WarehouseSet;
    static WarehouseSet newOrderRemoteWarehouses(Integer home_warehouse, const std::vector<NewOrderItem>& items);

	virtual void delivery(Integer warehouse_id, Integer carrier_id, 
        std::vector<DeliveryOrderInfo>* orders, TPCCUndo** undo);


	virtual void payment(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
            int32_t c_district_id, int32_t customer_id, float h_amount, TPCCUndo** undo);

    virtual void paymentHome(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
            int32_t c_district_id, int32_t c_id, float h_amount, TPCCUndo** undo);
    virtual void paymentRemote(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
            int32_t c_district_id, int32_t c_id, float h_amount, TPCCUndo** undo);
    virtual void paymentRemote(int32_t warehouse_id, int32_t district_id, int32_t c_warehouse_id,
            int32_t c_district_id, float h_amount, TPCCUndo** undo);

    Item* findItem(Integer i_id);

    Warehouse* findWarehouse(Integer w_id);

    Stock* findStock(Integer w_id, Integer s_id);

    District* findDistrict(Integer w_id, Integer d_id);

    Customer* findCustomer(Integer w_id, Integer d_id, Integer c_id);
    // Finds all customers that match (w_id, d_id, *, c_last), taking the n/2th one (rounded up).

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

}