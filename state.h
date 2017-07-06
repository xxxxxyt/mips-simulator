#ifndef _state_h
#define _state_h

#include <deque>
#include <cstdio>
#include <vector>
#include <map>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <atomic>
using namespace std;

class instruction;

const int MAXN = 1e6 + 5;
const int MAXL = 205;

extern atomic<int> reg[34]; // lo 32, hi 33
extern int heap_top, ins_top;
extern char mem[MAXN];
extern map<string, int> text_label, data_label;

extern vector<instruction*> ins_vec;

extern mutex mtx;
extern condition_variable jum;
extern condition_variable reg_taken[34];
extern condition_variable rep_empty[4];
extern deque<instruction*> rep[4];


#endif