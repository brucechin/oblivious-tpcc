#include <typeinfo>
#include "emp-tool/emp-tool.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include <iostream>
#include "clock.h"
using namespace std;
using namespace emp;


static string revealBinary(Integer &input, int length, int output_party) {
  bool * b = new bool[length];
  ProtocolExecution::prot_exec->reveal(b, output_party, (block *)input.bits,  length);
  char *bin = new char[length];

  for (int i=0; i<length; i++)
    bin[i] = (b[i] ? '1':'0');

  delete [] b;
  return string(bin);
  //return bin;
}

class Tuple{
public:
    int cols; // number of columns
    Integer* elements; //column data

    Tuple(){}

    Tuple(int c){
        cols = c;
        elements = new Integer[cols];
    }

    Tuple(vector<Integer> e){    
        cols = e.size();
        elements = new Integer[cols];
        for(int i = 0; i < cols; i++){
            elements[i] = e[i];
        }
    }

    ~Tuple(){
        delete elements;
    }

    Integer getElement(int index) const{
        assert(index >= 0 && index < cols);
        return elements[index];
    }

    void setElement(int index, Integer val){
        assert(index >= 0 && index < cols);
        elements[index] = val;
    }

    void If(const Bit& select, const Tuple& a) const{
        //if select is true, replace original tuple elements with a
        for(int i = 0; i < cols; i++){
            elements[i].If(select, a.elements[i]);
        }
    }

    void If(int index, const Bit& select, const Integer target) const{
        elements[index].If(select, target);
    }
};



// TODO: verify adding and subtracting two Integers
// TODO: Alice and Bob each generate N random random #s. find the maximum of the union of those values
int main(int argc, char** argv) {

  int party, port;
  static const vector<string> test{"hello", "alice", "bob"};
  string aliceHost = "127.0.0.1";

  parse_party_and_port(argv, 2, &party, &port);
  NetIO * io = new NetIO((party==ALICE ? nullptr : aliceHost.c_str()), port);
  setup_semi_honest(io, party);

  Integer a = Integer(32, 23, ALICE);
  Integer b = Integer(32, 24, BOB);
  Integer c = Integer(32, 1, PUBLIC);

  Tuple* d;
  vector<Tuple*> tuples1;
  vector<Tuple*> tuples2;
  SystemClock* clock = new SystemClock();

  Batcher batcher1, batcher2;
  int lenAlice = stoi(argv[3]);
  int lenBob = lenAlice;
  int64_t begin = clock->getMicroseconds();
  for(int i = 0; i < lenAlice; i++){
    for(int j = 0; j < 10; j++){
      batcher1.add<Integer>(32, 100 * i + j);
    }
  }

  for(int i = 0; i < lenBob; i++){
    for(int j = 0; j < 10; j++){
      batcher2.add<Integer>(32, 100 * i + j);
    }
  }
  



  batcher1.make_semi_honest(ALICE);
	batcher2.make_semi_honest(BOB);



  for(int i = 0; i < lenAlice; i++){
    d = new Tuple(10);
    for(int j = 0; j < 10; j++){
      d->setElement(j, batcher1.next<Integer>());
    }
    tuples1.push_back(d);
  }

  for(int i = 0; i < lenBob; i++){
    d = new Tuple(10);
    for(int j = 0; j < 10; j++){
      d->setElement(j, batcher2.next<Integer>());
    }
    tuples1.push_back(d);
  }

  io->flush();
  int64_t end = clock->getMicroseconds();
  cout << "batcher encryption time : " << end - begin << " ms" <<endl;



  begin = clock->getMicroseconds();
  for(int i = 0; i < lenAlice; i++){
    d = new Tuple(10);
    for(int j = 0; j < 10; j++){
      d->setElement(j, Integer(32, 100 * i + j, ALICE));
    }
    tuples2.push_back(d);
  }

  for(int i = 0; i < lenBob; i++){
    d = new Tuple(10);
    for(int j = 0; j < 10; j++){
      d->setElement(j, Integer(32, 100 * i + j, BOB));
    }
    tuples2.push_back(d);
  }
  io->flush();
  end = clock->getMicroseconds();
  cout << "one-by-one encryption time : " << end - begin << " micro second" <<endl;


  for(int i = 0; i < lenAlice; i++){
    for(int j = 0; j < 10; j++){
      cout << tuples1[i]->getElement(j).reveal<int>(PUBLIC) << " " << tuples2[i]->getElement(j).reveal<int>(PUBLIC) <<endl;
      tuples1[i]->setElement(j, tuples1[i]->getElement(j) + Integer(32, 1, PUBLIC));
      tuples2[i]->setElement(j, tuples2[i]->getElement(j) + Integer(32, 1, PUBLIC));

      cout << tuples1[i]->getElement(j).reveal<int>(PUBLIC) << " " << tuples2[i]->getElement(j).reveal<int>(PUBLIC) <<endl;

      if((tuples1[i]->getElement(j) != tuples2[i]->getElement(j)).reveal<bool>(PUBLIC)){
        cout << "error at " << i << " " << j << endl;
      }
    }
  }
  // for(int i = 0; i < lenAlice + lenBob; i++){
  //   for(int j = 0; j < 10; j++){
  //     cout << tuples[i]->getElement(j).reveal<int>(PUBLIC) << " ";
  //   }
  //   cout << endl;
  // }



  io->flush();

  // secret share bits

  delete io;

}
