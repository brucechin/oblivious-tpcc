#include <iostream>
#include <mutex> // For std::unique_lock
#include <shared_mutex>
#include <thread>
#include <typeinfo>
#include "emp-tool/emp-tool.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include <iostream>
#include "clock.h"
#include "tpccdb_tmp.h"
#include "ctpl.h"

using namespace emp;
using namespace std;

class ThreadSafeCounter
{
public:
    ThreadSafeCounter() = default;

    // Multiple threads/readers can read the counter's value at the same time.
    Integer get() const
    {
        std::shared_lock lock(mutex_);
        return value_;
    }

    // Only one thread/writer can increment/write the counter's value.
    void increment()
    {
        std::unique_lock lock(mutex_);
        value_ = value_ + Integer(32, 1, PUBLIC);
    }

private:
    mutable std::shared_mutex mutex_;
    Integer value_ = Integer(32, 0, PUBLIC);
};

int main(int argc, char **argv)
{
    
    int party, port;
    string aliceHost = "127.0.0.1";
    vector<std::thread> pool;
    parse_party_and_port(argv, 2, &party, &port);
    NetIO *io = new NetIO((party == ALICE ? nullptr : aliceHost.c_str()), port);
    setup_semi_honest(io, party);
    int NUM_TRANSACTIONS = stoi(argv[3]);
    SystemClock* clock = new SystemClock();
    ThreadSafeCounter counter;
    auto increment_and_print = [&counter]() {
        for (int i = 0; i < 100000; i++)
        {
            counter.increment();
            //std::cout << std::this_thread::get_id() << ' ' << counter.get().reveal<int>(PUBLIC) << '\n';

            // Note: Writing to std::cout actually needs to be synchronized as well
            // by another std::mutex. This has been omitted to keep the example small.
        }
    };

    int64_t begin = clock->getMicroseconds();
    for (int i = 0; i < NUM_TRANSACTIONS; ++i)
    {
        //cout << i << "th transaction start \n\n\n"<<endl;
        //pool.push_back(std::thread(increment_and_print));
    }

    for (int i = 0; i < NUM_TRANSACTIONS; ++i)
    {
        //pool[i].join();
        //cout << i << "th txn joins" << endl;
    }

    int64_t end = clock->getMicroseconds();
    int64_t microseconds = end - begin;
    printf("concurrent Integer increment %d transactions in %d ms = %f txns/s\n", NUM_TRANSACTIONS,
           (microseconds + 500) / 1000, NUM_TRANSACTIONS / (double)microseconds * 1000000.0);
    io->flush();
}