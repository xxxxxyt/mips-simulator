#include <memory>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include "state.h"
#include "instruction.hpp"
#include "interpreter.h"
using namespace std;

typedef unsigned long long ull;
atomic<int> reg[34]; // lo 32, hi 33
int heap_top = 0, ins_top = 0;
char mem[MAXN];
map<string, int> text_label, data_label;

vector<instruction*> ins_vec;

mutex mtx;
condition_variable jum;
condition_variable reg_taken[34];
condition_variable rep_empty[4];
deque<instruction*> rep[4];

void shut_down(int val) {
	vector<instruction*>::iterator it = ins_vec.begin();
	while (it != ins_vec.end()) delete *(it++);
	//while (true);
	exit(val);
}

int main(int argc, char *argv[]) {
//int main() {

	ifstream source;
	source.open(argv[1]);
	interpreter itp(source, cin, cout);
	itp.interprete();
	shut_down(0);

	return 0;
}