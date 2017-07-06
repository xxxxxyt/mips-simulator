#ifndef _interpreter_h
#define _interpreter_h

#include "state.h"
#include "instruction.hpp"
using namespace std;

class interpreter {
public:
	ifstream &src;
	istream &is;
	ostream &os;

	interpreter(ifstream &_src, istream &_is, ostream &_os);
	void interprete();
	void read_in();
	void execute_text();
};

#endif