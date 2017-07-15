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
int reg[34]; // lo 32, hi 33
int heap_top = 0, ins_top = 0;
char mem[MAXN];
map<string, int> text_label, data_label;

vector<instruction*> ins_vec;
instruction *plat[5];

int wrong_cnt = 0, predict_cnt = 0;

void shut_down(int val) {
	vector<instruction*>::iterator it = ins_vec.begin();
	while (it != ins_vec.end()) delete *(it++);
	//cout << endl << 1.00 - 1.00 * wrong_cnt / predict_cnt << " in " << predict_cnt << endl;
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