#ifndef _tool_h
#define _tool_h
#include <deque>
#include <string>
using namespace std;

int string_to_int(string str);

const deque<string> REG_STR = {"$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31", "$lo", "$hi"};
const deque<string> REG_NUM = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra", "$lo", "$hi"};
int string_to_reg(string str);
int power_2(int n);
string get_phrase(char *str, int &i, int l);
string get_str(char *str, int &i, int l);

#endif