#pragma once
#include "assert.h"
#include "btree.h"
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct District {

	// int d_id;
	// int d_w_id;
	// int d_next_o_id;
	// float d_ytd;
	// float d_tax;
	// String d_name;
	// String d_street_1;
	// String d_street_2;
	// String d_city;
	// String d_state;
	// String d_zip;

};

struct History {

	// int h_c_id;
	// int h_c_d_id;
	// int h_c_w_id;
	// int h_d_id;
	// int h_w_id;
	// Timestamp h_date;
	// float h_amount;
	// String h_data;

};

struct Item {

	// int i_id; // PRIMARY KEY
	// int i_im_id;
    // double i_price;
	// String i_name;
	// String i_data;

};

struct NewOrder {

	// int no_w_id;
	// int no_d_id;
	// int no_o_id;

};

struct Order {

	// int o_id;
	// int o_w_id;
	// int o_d_id;
	// int o_c_id;
	// Integer o_carrier_id;
	// int o_ol_cnt;
	// int o_all_local;
	// Timestamp o_entry_d;

};

struct Stock {

	// int s_i_id; // PRIMARY KEY 2
	// int s_w_id; // PRIMARY KEY 1
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

};

struct Customer {

	// int c_id;
	// int c_d_id;
	// int c_w_id;
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

};

struct Warehouse {

	// int w_id; // PRIMARY KEY
	// float w_ytd;
    // double w_tax;
	// String w_name;
	// String w_street_1;
	// String w_street_2;
	// String w_city;
	// String w_state;
	// String w_zip;
};

struct OrderLine {

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
};


class TPCCDB{
    private:
    //B+ tree should be used instead of vector
    // vector<Item> items;
    // vector<District> districts;
    // vector<History> histories;
    // vector<NewOrder> neworders;
    // vector<Oorder> oorders;
    // vector<Stock> stocks;
    // vector<Customer> customers;
    // vector<Warehouse> warehouses;
    // vector<OrderLine> orderlines;

    std::vector<Item> items_;
    std::vector<const History*> history_;
    BPlusTree<int32_t, Warehouse*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> warehouses_;
    BPlusTree<int32_t, Stock*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> stock_;
    BPlusTree<int32_t, District*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> districts_;
    BPlusTree<int32_t, Customer*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> customers_;
    BPlusTree<int32_t, Order*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> orders_;
    BPlusTree<int32_t, OrderLine*, KEYS_PER_INTERNAL, KEYS_PER_LEAF> orderlines_;
    typedef std::map<int64_t, NewOrder*> NewOrderMap;
    NewOrderMap neworders_;
    

    public:
    TPCCDB();
    ~TPCCDB();


}