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
	atomic<int> jump_cnt;
	atomic<instruction*> rep[4];

	interpreter(ifstream &_src, istream &_is, ostream &_os);
	void interprete();
	void read_in();

	void write_back();
	void memory_access();
	void execute();
	void data_prepare();
	void instruction_fetch();
	void run();
};

#endif