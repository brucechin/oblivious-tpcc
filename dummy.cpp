#include <typeinfo>
#include "emp-tool/emp-tool.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include <iostream>
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




// TODO: verify adding and subtracting two Integers
// TODO: Alice and Bob each generate N random random #s. find the maximum of the union of those values
int main(int argc, char** argv) {

  int party, port;

  string aliceHost = "127.0.0.1";

  parse_party_and_port(argv, 2, &party, &port);
  NetIO * io = new NetIO((party==ALICE ? nullptr : aliceHost.c_str()), port);
  setup_semi_honest(io, party);

  Integer a = Integer(32, 23, ALICE);
  Integer b = Integer(32, 24, BOB);
  Integer c = Integer(32, 1, PUBLIC);
  for(int i = 0; i < 100; i++){
    a = a + c;
    cout << a.reveal<int>(PUBLIC) << endl;
  }
  // for(int i = 0; i < 100; i++){
  //     Integer b = Integer(32, i, BOB);
  //     Bit tru = Bit(true, PUBLIC);
  //     Bit fal = Bit(false, PUBLIC);
  //     Bit res = If(a == b, tru, fal);
  //     cout << i << " " << res.reveal<bool>(PUBLIC) << endl; 
  // }

  io->flush();

  // secret share bits

  delete io;

}
