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
    vector<Integer> elements; //column data

    

    Tuple(vector<Integer>& e){
        elements = e;
        cols = elements.size();
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

    Table(vector<string>& f){
        rows = 0;
        fields = f;
        for(int i = 0; i < fields.size(); i++){
            offset[fields[i]] = i;
        }
    }

    Table(vector<string>& f, vector<Tuple*> t){
        rows = t.size();
        fields = f;
        for(int i = 0; i < fields.size(); i++){
            offset[fields[i]] = i;
        }
        tuples = t;
    }

    void insertTuple(Tuple* t){

    }

    Tuple* getTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields){

    }

    bool removeTupleByPrimaryKey(vector<Integer> keys, vector<string> keyFields){

    }

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


}