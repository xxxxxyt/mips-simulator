#include "stdafx.h"
#include <memory>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <stdlib.h>
using namespace std;

const int MAXN = 1e6 + 5;

int reg[34]; // lo 32, hi 33
char mem[MAXN];
int heap_top = 0;

// ============================== tools =============================
int string_to_int(const string &str) {
	int res = 0;
	int i = str.length() - 1;
	while (i >= 0 && (str[i] < '0' || str[i] > '9')) --i;
	while (i >= 0 && (str[i] >= '0'  && str[i] <= '9'))
		res *= 10, res += str[i--] - '0';
	if (i >= 0 && str[i] == '-') res = -res;
	return res;
}
const string REG_STR[] = {"$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31", "$lo", "$hi"};
const string REG_NUM[] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra", "$lo", "$hi"};
int string_to_reg(string str) {
	if (str.empty()) return -1;
	if (str.back() == ',') str.pop_back();
	for (int i = 0; i < 34; ++i) if (str == REG_STR[i] || str == REG_NUM[i]) return i;
	return -1;
}
int power_2(int n) {
	int res = 1;
	while (n--) res <<= 1;
	return res;
}
string get_phrase(char *str, int &i, int l) {
	string res = "";
	while (i < l && str[i] != ' ') res += str[i++];
	return res;
}
string get_str(char *str, int &i, int l) {
	string res = "";
	while (i < l) {
		res += str[i++];
		if (res.back() == '\\') {
			char ch = str[i++], real;
			switch (ch) {
			case 'n': real = '\n'; break;
			case 'r': real = '\r'; break;
			case 't': real = '\t'; break;
			case '\\': real = '\\'; break;
			case '\'': real = '\''; break;
			case '\"': real = '\"'; break;
			case '0': real = '\0'; break;
			}
			res.back() = real;
		}
	}
	return res;
}




// ========================= instructions ======================
class instruction {
public:
	int status;

	instruction() : status(0) {}
	virtual instruction* copy() { return new instruction(*this); }
	virtual void data_prepare() {}
	virtual void execute() {}
	virtual void memory_access() {}
	virtual void write_back() {}
	virtual ~instruction() {}
};

class loading : public instruction {
public:
	int rdset;
	string address;
	int pos, val;

	loading(const string &_rdset, const string &_address) :
		instruction(), rdset(string_to_reg(_rdset)), address(_address), val(0) {}
	virtual instruction* copy() { return new loading(*this); }
	virtual void execute() {
		int i = address.find('(');
		int j = address.find(')');
		int offset = string_to_int(address.substr(0, i));
		int rsrc = string_to_reg(address.substr(i + 1, j));
		pos = reg[rsrc] + offset;
	}
	virtual void write_back() { reg[rdset] = val; }
};
class la : public loading {
public:
	la(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new la(*this); }
	virtual void write_back() { reg[rdset] = pos; }
};
class lb : public loading {
public:
	lb(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new lb(*this); }
	virtual void memory_access() { val |= mem[pos]; }
};
class lh : public loading {
public:
	lh(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new lh(*this); }
	virtual void memory_access() {
		val |= mem[pos];
		val |= (mem[pos + 1] << 8);
	}
};
class lw : public loading {
public:
	lw(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new lw(*this); }
	virtual void memory_access() {
		for (int k = 0, t = 0; k < 4; ++k, t += 8)
			val |= (mem[pos + k] << t);
	}
};

class storing : public instruction {
public:
	int rsrc;
	string address;
	int pos, val;
	
	storing(const string &_rsrc, const string &_address) :
		instruction(), rsrc(string_to_reg(_rsrc)), address(_address), val(0) {
	}
	virtual instruction* copy() { return new storing(*this); }
	virtual void data_prepare() { val = reg[rsrc]; }
	virtual void execute() {
		int i = address.find('(');
		int j = address.find(')');
		int offset = string_to_int(address.substr(0, i));
		int rsrc1 = string_to_reg(address.substr(i + 1, j));
		pos = reg[rsrc1] + offset;
	}
};
class sb : public storing {
public:
	sb(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sb(*this); }
	virtual void memory_access() { mem[pos] = (char)(val & 0xff); }
};
class sh : public storing {
public:
	sh(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sh(*this); }
	virtual void memory_access() {
		mem[pos] = (char)(val & 0xff);
		mem[pos + 1] = (char)((val & 0xff00) >> 8);
	}
};
class sw : public storing {
public:
	sw(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sw(*this); }
	virtual void memory_access() {
		mem[pos] = (char)(val & 0xff);
		mem[pos + 1] = (char)((val & 0xff00) >> 8);
		mem[pos + 2] = (char)((val & 0xff0000) >> 16);
		mem[pos + 3] = (char)((val & 0xff000000) >> 24);
	}
};

class assignment : public instruction {
public:
	int rdset, rsrc, imm;

	assignment(const string &_rdset, const string &_rsrc) {
		rdset = string_to_reg(_rdset);
		rsrc = string_to_reg(_rsrc);
		if (rsrc == -1) imm = string_to_int(_rsrc);
	}
	virtual instruction* copy() { return new assignment(*this); }
	virtual void data_prepare() {if (rsrc != -1) imm = reg[rsrc];}
	virtual void write_back() { reg[rdset] = imm; }
};
class mfhi : public assignment {
public:
	mfhi(const string &_rdset) : assignment(_rdset, "") {}
	virtual instruction* copy() { return new mfhi(*this); }
	virtual void data_prepare() { imm = reg[33]; }
};
class mflo : public assignment {
public:
	mflo(const string &_rdset) : assignment(_rdset, "") {}
	virtual instruction* copy() { return new mflo(*this); }
	virtual void data_prepare() { imm = reg[32]; }
};

class calculation : public instruction {
public:
	int rdset, rsrc1, rsrc2;
	int imm1, imm2, res;

	calculation(const string &_rdset, const string &_rsrc1, const string &_rsrc2) {
		rdset = string_to_reg(_rdset);
		rsrc1 = string_to_reg(_rsrc1);
		rsrc2 = string_to_reg(_rsrc2);
		if (rsrc1 == -1) imm1 = string_to_int(_rsrc1);
		if (rsrc2 == -1) imm2 = string_to_int(_rsrc2);
	}
	virtual instruction* copy() { return new calculation(*this); }
	virtual void data_prepare() {
		if (rsrc1 != -1) imm1 = reg[rsrc1];
		if (rsrc2 != -1) imm2 = reg[rsrc2];
	}
	virtual void write_back() { reg[rdset] = res; }
};
class add : public calculation { // & addu, addiu
public:
	bool unsign;

	add(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new add(*this); }
	virtual void execute() { res = imm1 + imm2; }
};
class sub : public calculation { // & subu
public:
	bool unsign;

	sub(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new sub(*this); }
	virtual void execute() { res = imm1 - imm2;}
};
class mul : public calculation {
public:
	int para;
	bool unsign;
	long long llres;

	mul(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), para(ph3 == "" ? 2 : 3), unsign(_unsign) {}
	virtual instruction* copy() { return new mul(*this); }
	virtual void data_prepare() {
		if (para == 2) {
			imm2 = imm1;
			imm1 = reg[rdset];
			if (rsrc1 != -1) imm2 = reg[rsrc1];
		}
		else {
			if (rsrc1 != -1) imm1 = reg[rsrc1];
			if (rsrc2 != -1) imm2 = reg[rsrc2];
		}
	}
	virtual void execute() { // confusing...
		if (unsign) llres = (unsigned long long)1 * (unsigned int)(imm1) * (unsigned int)(imm2);
		else llres = 1LL * imm1 * imm2; 
	}
	virtual void write_back() {
		if (para == 3) reg[rdset] = llres;
		else {
			reg[32] = (llres & 0xffffffff); // lo
			reg[33] = ((llres & 0xffffffff00000000) >> 32); // hi
		}
	}
};
class div : public calculation { // & divu
public:
	int para, q, r;
	bool unsign;

	div(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), para(ph3 == "" ? 2 : 3), unsign(_unsign) {}
	virtual instruction* copy() { return new div(*this); }
	virtual void data_prepare() {
		if (para == 2) {
			imm2 = imm1;
			imm1 = reg[rdset];
			if (rsrc1 != -1) imm2 = reg[rsrc1];
		}
		else {
			if (rsrc1 != -1) imm1 = reg[rsrc1];
			if (rsrc2 != -1) imm2 = reg[rsrc2];
		}
	}
	virtual void execute() {
		if (unsign) q = (unsigned int)(imm1) / (unsigned int)(imm2);
		else q = imm1 / imm2;
		if (unsign) r = (unsigned int)(imm1) % (unsigned int)(imm2);
		else r = imm1 % imm2;
	}
	virtual void write_back() {
		if (para == 3) reg[rdset] = q;
		else {
			reg[32] = q;
			reg[33] = r;
		}
	}
};
class xor : public calculation { // & xoru
public:
	bool unsign;

	xor(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new xor(*this); }
	virtual void execute() { res = imm1 ^ imm2;}
};
class neg : public calculation { // & negu
public:
	bool unsign;

	neg(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new neg(*this); }
	virtual void execute() { res = ~imm1; }
};
class rem : public calculation { // & remu
public:
	bool unsign;

	rem(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new rem(*this); }
	virtual void execute() { 
		if (unsign) res = (unsigned int)imm1 % (unsigned int)imm2;
		else res = imm1 % imm2; 
	}
};



// ============================= interpreter ==================================
class interpreter {
private:
	vector<instruction*> ins_vec;
	map<string, int> text_label, data_label;

public:
	interpreter() { reg[29] = MAXN - 1; } // $sp stack_top
	void interprete(ifstream &source, ifstream &input) {
		read_in(source);
		execute_text(input);
	}
	void read_in(ifstream &ifs) {
		const int MAXL = 105;
		char str[MAXL];
		bool text_block = false;
		while (ifs.getline(str, MAXL, '\n')) {
			string tmp = "";
			int i = 0, l = strlen(str);
			while (str[i] == ' ' || str[i] == '\t') ++i;
			if (str[i] == '.') { // .xxx
				++i;
				tmp = get_phrase(str, i, l);
				if (tmp == "align") {
					++i;
					tmp = get_phrase(str, i, l);
					int n = string_to_int(tmp);
					n = power_2(n);
					heap_top += (n - heap_top  % n) % n;
				}
				else if (tmp == "ascii" || tmp == "asciiz") {
					i += 2;
					tmp = get_str(str, i, l - 1);
					for (int i = 0; i < tmp.length(); ++i)
						mem[heap_top++] = tmp[i];
					if (tmp == "asciiz") mem[heap_top++] = '\0';
				}
				else if (tmp == "byte" || tmp == "half" || tmp == "word") {
					int m = tmp == "byte" ? 1 : (tmp == "half" ? 2 : 4);
					while (true) {
						if (i == l) break;
						++i;
						tmp = get_phrase(str, i, l);
						int n = string_to_int(tmp);
						mem[heap_top++] = (char)(n & 0xff);
						if (m > 1) mem[heap_top++] = (char)((n & 0xff00) >> 8);
						if (m > 2) mem[heap_top++] = (char)((n & 0xff0000) >> 16);
						if (m > 2) mem[heap_top++] = (char)((n & 0xff000000) >> 24);
					}
				}
				else if (tmp == "space") {
					++i;
					tmp = get_phrase(str, i, l);
					int n = string_to_int(tmp);
					heap_top += n;
				}
				else if (tmp == "data" || tmp == "text") {
					text_block = tmp == "text";
				}
			}
			else if (str[l - 1] == ':') { // label:
				string tmp = get_phrase(str, i, l - 1);
				if (text_block) text_label[tmp] = ins_vec.size();
				else data_label[tmp] = heap_top;
			}
			else { // code in text
				//cout << str << endl;
				string name = get_phrase(str, i, l); ++i;
				string ph1, ph2, ph3;
				ph1 = get_phrase(str, i, l); ++i;
				ph2 = get_phrase(str, i, l); ++i;
				ph3 = get_phrase(str, i, l); ++i;
				//cout << name << " " << ph1 << " " << ph2 << " " << ph3 << endl;
				instruction *ptr;
				// loading instruction
				if (name == "la") ptr = new la(ph1, ph2);
				if (name == "lb") ptr = new lb(ph1, ph2);
				if (name == "lh") ptr = new lh(ph1, ph2);
				if (name == "lw") ptr = new lw(ph1, ph2);
				// storing instruction
				if (name == "sb") ptr = new sb(ph1, ph2);
				if (name == "sh") ptr = new sh(ph1, ph2);
				if (name == "sw") ptr = new sw(ph1, ph2);
				// assignment instruction
				if (name == "li" || name == "move") ptr = new assignment(ph1, ph2);
				if (name == "mfhi") ptr = new mfhi(ph1);
				if (name == "mflo") ptr = new mflo(ph1);
				// calculation instruction

				// jump instruction
			}
		}
	}
	void execute_text(ifstream &input) {}
	~interpreter() {
		vector<instruction*>::iterator it = ins_vec.begin();
		while (it != ins_vec.end()) delete *(it++);
	}
};

int main() {

	ifstream source;
	ifstream input;
	
	source.open("gcd-5090379042-jiaxiao.s");
	if (!source) cout << "Fail to open file!\n";
	
	interpreter itp;
	itp.interprete(source, input);

	source.close();
	input.close();
	return 0;
}