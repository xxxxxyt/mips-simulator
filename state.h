#ifndef _state_h
#define _state_h

#include <cstdio>
#include <vector>
#include <map>
using namespace std;

class instruction;

const int MAXN = 1e6 + 5;
const int MAXL = 205;

extern int reg[34]; // lo 32, hi 33
extern int heap_top, ins_top;
extern char mem[MAXN];
extern map<string, int> text_label, data_label;

extern vector<instruction*> ins_vec;
extern instruction *plat[5];

#endif