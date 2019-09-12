#include "tpccdb.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <vector>
#include "assert.h"

using std::vector;



template <typename KeyType, typename ValueType>
static void deleteBTreeValues(BPlusTree<KeyType, ValueType*, TPCCTables::KEYS_PER_INTERNAL, TPCCTables::KEYS_PER_LEAF>* btree) {
    KeyType key = std::numeric_limits<KeyType>::max();
    ValueType* value = NULL;
    while (btree->findLastLessThan(key, &value, &key)) {
        assert(value != NULL);
        delete value;
    }
}

TPCCDB::~TPCCDB() {
    // Clean up the b-trees with this gross hack
    deleteBTreeValues(&warehouses_);
    deleteBTreeValues(&stock_);
    deleteBTreeValues(&districts_);
    deleteBTreeValues(&orders_);
    deleteBTreeValues(&orderlines_);

    STLDeleteValues(&neworders_);
    STLDeleteElements(&customers_by_name_);
    STLDeleteElements(&history_);
}

