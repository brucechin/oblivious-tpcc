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

  // secret share bits
  Integer a =  Integer(32, 6400000, ALICE);
  cout << "Alice secret shared: " << revealBinary(a, 32, XOR) << endl;
  Integer b =  Integer(32, 6400001, BOB);
  cout << "Bob secret shared: " << revealBinary(b, 32, XOR) << endl;

  Bit result = a >= b;
  io->flush();


  bool trueResult = result.reveal(PUBLIC);
  
  cout << "Is alice richer? " << trueResult << endl;

  delete io;
}
