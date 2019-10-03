
#define __STDC_FORMAT_MACROS
#include <climits>
#include <cstdio>
#include <inttypes.h>
#include "clock.h"
#include "randomgenerator.h"
#include "tpccclient.h"
#include "tpccdb.h"
#include <unistd.h>
#include <cstring>
#include<sstream>
#include<string>
#include<iostream>
#include<fstream>

using namespace emp;
using namespace std;

static const int NUM_TRANSACTIONS = 10000;

string getDirectory(){
    char abs_path[1024];
    int cnt = readlink("/proc/self/exe", abs_path, 1024);//获取可执行程序的绝对路径
    if(cnt < 0|| cnt >= 1024)
    {
        return NULL;
    }

    //最后一个'/' 后面是可执行程序名，去掉devel/lib/m100/exe，只保留前面部分路径
    for(int i = cnt; i >= 0; --i)
    {
        if(abs_path[i]=='/')
        {
            abs_path[i + 1]='\0';
            break;
        }
    }

    string path(abs_path);

    return path;
}

void printFiles(string dir){
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL)
    {
        cout << "Can't open " << dir << endl;
    }
    while((dirp = readdir(dp)) != NULL)
    {
        cout << dirp->d_name << endl;
    }
    closedir(dp);
}

int numOfRowsss(string filename){
    //count lines of the csv file
    int res = 0;
    std::ifstream f(filename, ios::in);
    std::string line;
	cout << f.is_open()<<endl;
    while(getline(f, line, '\n')){
        res++;
    }
    f.close();
    return res;

}


int main(int argc, char** argv) {
    
    int party, port;
    int num_warehouses = 2;
    parse_party_and_port(argv, 2, &party, &port);
    
    NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);
	setup_semi_honest(io, party);
    TPCCDB* tables = new TPCCDB(party, port);
    //printFiles("/home/liankeqin/emp-readme/scripts/emp-ot/bin/data");
    //cout << getDirectory() << endl;
    //cout <<"My party is :" <<  tables->party_ << endl;
    string prefix = "./test/smalldata/";
    tables->loadItems(prefix + "itemAlice.csv", prefix + "itemBob.csv");
    tables->loadStocks(prefix + "stockAlice.csv", prefix + "stockBob.csv");
    tables->loadWarehouses(prefix + "warehouseAlice.csv", prefix + "warehouseBob.csv");
    tables->loadDistricts(prefix + "districtAlice.csv", prefix + "districtBob.csv");
    tables->loadCustomers(prefix + "customerAlice.csv", prefix + "customerBob.csv");
    //tables->loadOrders(prefix + "oorderAlice.csv", prefix + "oorderBob.csv");
    //tables->loadOrderLines(prefix + "order_lineAlice.csv", prefix + "order_lineBob.csv");
    //tables->loadNewOrders(prefix + "new_orderAlice.csv", prefix + "new_orderBob.csv");

    cout << "item rows : " << tables->items_.size()<<endl;
    cout << "stock rows : " << tables->stocks_.size() <<endl;
    SystemClock* clock = new SystemClock();
    

    tpcc::RealRandomGenerator* random = new tpcc::RealRandomGenerator();
    tpcc::NURandC cLoad = tpcc::NURandC::makeRandom(random);
    random->setC(cLoad);

    cout << "random generator init "<<endl;
    // Client owns all the parameters
    TPCCClient client(clock, random, tables);
    cout << "tpcc client init" <<endl;



    int64_t begin = clock->getMicroseconds();

    for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
        client.doNewOrder();
        cout << i << "th new order transaction finish \n\n\n"<<endl;

        if(i % 10 == 0){
            for(int i = 0; i < client.db_->districts_.size(); i++){
                client.db_->districts_[i]->print();
            }
        }
        io->flush();
    }
    
    delete io;
    

    int64_t end = clock->getMicroseconds();
    int64_t microseconds = end - begin;
    printf("%d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
           (microseconds + 500)/1000, NUM_TRANSACTIONS / (double) microseconds * 1000000.0);

    return 0;
}