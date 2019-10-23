#include <typeinfo>
#include "emp-tool/emp-tool.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include <iostream>
#include "clock.h"
#include "tpccdb_tmp.h"
#include <inttypes.h>
#include "ThreadPool/ThreadPool.h"
using namespace std;
using namespace emp;

#define MAX_VALUE 10000

class ConcurrentTest
{
public:
  vector<vector<Integer>> tuples;
  vector<vector<int>> intList;
  Batcher batcher1, batcher2;
  int lenAlice = MAX_VALUE;
  int lenBob = lenAlice;
  std::array<std::shared_mutex, MAX_VALUE * 2> mutexes;
  ConcurrentTest()
  {
    tuples = vector(MAX_VALUE * 2, vector<Integer>(10, Integer(INT_LENGTH, 0, PUBLIC)));
    intList = vector(MAX_VALUE * 2, vector<int>(10, 0));

    for (int i = 0; i < lenAlice + lenBob; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        intList[i][j] = i;
      }
    }

    for (int i = 0; i < lenAlice; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        batcher1.add<Integer>(32, 100 * i + j);
      }
    }

    for (int i = 0; i < lenBob; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        batcher2.add<Integer>(32, 100 * i + j);
      }
    }

    batcher1.make_semi_honest(ALICE);
    batcher2.make_semi_honest(BOB);

    for (int i = 0; i < lenAlice; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        tuples[i][j] = batcher1.next<Integer>();
      }
    }

    for (int i = 0; i < lenBob; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        tuples[i][j] = batcher2.next<Integer>();
      }
    }
  }

  int intSearch()
  {
    int a = 1;
    for (int i = 0; i < 1000; i++)
    {
      
      for (int j = 0; j < MAX_VALUE; j++)
      {
        //std::unique_lock lock(mutexes[j]);
        for (int k = 0; k < 10; k++)
        {
          int tmp = intList[j][k];
          tmp += a;
          //cout << std::this_thread::get_id() << " yes" <<endl;
        }
      }
    }
    return 1;
  }

  int IntegerSearch()
  {
    Integer a = Integer(INT_LENGTH, 1, PUBLIC);
    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j < MAX_VALUE; j++)
      {
        //std::unique_lock lock(mutexes[j]);
        for (int k = 0; k < 10; k++)
        {
          Integer tmp = tuples[j][k];
          tmp = tmp + a;
          //cout << std::this_thread::get_id() << " yes" <<endl;
        }
      }
    }
    return 2;
  }
  
};

Table *items_ = new Table(vector<string>{"i_id", "i_price", "i_im_id"});
vector<string> itemPrimaryKeyNames_{"i_id"};


void test()
{

  for (int i = 1; i < 100; ++i)
  {
    Integer a = Integer(INT_LENGTH, i, PUBLIC);
    Tuple test = items_->getTupleByPrimaryKey(vector<Integer>{a}, itemPrimaryKeyNames_);
  }
}

int main(int argc, char **argv)
{

  int party, port;
  string aliceHost = "127.0.0.1";

  parse_party_and_port(argv, 2, &party, &port);
  NetIO *io = new NetIO((party == ALICE ? nullptr : aliceHost.c_str()), port);
  setup_semi_honest(io, party);
  int NUM_TRANSACTIONS = stoi(argv[3]);
  string dataFilePath = "./test/data/";
  string fileAlice = "Alice.csv";
  string fileBob = "Bob.csv";
  //items_->loadFromCSVBatcher(dataFilePath + "item" + fileAlice, dataFilePath + "item" + fileBob, vector<int>({0, 2, 4}),vector<int>({1, 100, 1}), party);

  vector<std::thread> pool;
  SystemClock *clock = new SystemClock();

  
  ThreadPool threadpool(8);
  vector<std::future<int>> results;
  //threadpool.init();
  ConcurrentTest* t = new ConcurrentTest();
  int64_t begin = clock->getMicroseconds();
  for (int i = 0; i < NUM_TRANSACTIONS; ++i)
  {
    //cout << i << "th transaction start \n\n\n"<<endl;
    //pool.push_back(std::thread(&ConcurrentTest::IntegerSearch, t));
    results.emplace_back(threadpool.enqueue(&ConcurrentTest::IntegerSearch, t));
  }

  for (int i = 0; i < NUM_TRANSACTIONS; ++i)
  {
    cout << results[i].get();
    //pool[i].join();
    //cout << i << "th txn joins" << endl;
  }

  int64_t end = clock->getMicroseconds();
  int64_t microseconds = end - begin;
  printf("\n int linear search %d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
         (microseconds + 500) / 1000, NUM_TRANSACTIONS / (double)microseconds * 1000000.0);

  // begin = clock->getMicroseconds();
  // for (int i = 0; i < NUM_TRANSACTIONS; ++i)
  // {
  //   //cout << i << "th transaction start \n\n\n"<<endl;
  //   //pool.push_back(std::thread(&ConcurrentTest::IntegerSearch, t));
  //   results.emplace_back(threadpool.enqueue(&ConcurrentTest::IntegerSearch, t));
  // }

  // for (int i = 0; i < NUM_TRANSACTIONS; ++i)
  // {
  //   cout << results[i].get();
  //   //pool[i].join();
  //   //cout << i << "th txn joins" << endl;
  // }

  // end = clock->getMicroseconds();
  // microseconds = end - begin;
  // printf("\n int linear search %d transactions in %" PRId64 " ms = %f txns/s\n", NUM_TRANSACTIONS,
  //        (microseconds + 500) / 1000, NUM_TRANSACTIONS / (double)microseconds * 1000000.0);
  // io->flush();


  // secret share bits

  delete io;
}
