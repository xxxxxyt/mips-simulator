#ifndef _interpreter_h
#define _interpreter_h

#include <thread>
#include "state.h"
#include "instruction.hpp"
using namespace std;

const int MOD = 256, S = 8;

class interpreter {
public:
	ifstream &src;
	istream &is;
	ostream &os;
	int cnt[MOD][S], status;
	bool branch_in = false, predict[MOD][1 << S];
	int ins_vec_sz, rec_ins_top;
	int reg_cnt[34];

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