#include <memory>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
using namespace std;

typedef unsigned long long ull;

const int MAXN = 1e6 + 5;
const int MAXL = 205;

int reg[34]; // lo 32, hi 33
int heap_top = 0, ins_top = 0;
char mem[MAXN];
map<string, int> text_label, data_label;

// ============================== tools =============================
int string_to_int(string str) {
	int res = 0;
	int l = str.length(), i = 0;
	bool flag = false;
	while (i < l && (str[i] < '0' || str[i] > '9')) flag = str[i] == '-', ++i;
	while (i < l && (str[i] >= '0'  && str[i] <= '9'))
		res *= 10, res += str[i++] - '0';
	if (flag) res = -res;
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
void shut_down(int val);

// ========================= instructions ======================
class instruction {
public:
	vector<int> reg_to_read, reg_to_write;
	int jump_type; // 0 not jump; 1 jump; 2 branch
	bool write_to_mem;

	instruction() : jump_type(0), write_to_mem(false) {
		reg_to_read.clear();
		reg_to_write.clear();
	}
	virtual instruction* copy() { return new instruction(*this); }
	virtual void data_prepare() {}
	virtual void execute() {}
	virtual void memory_access() {}
	virtual void write_back() {}
	virtual ~instruction() {}
};
vector<instruction*> ins_vec;
instruction *plat[5];

class loading : public instruction {
public:
	int rdset, rsrc;
	string address;
	int val, pos, offset;

	loading(const string &_rdset, const string &_address) :
		instruction(), rsrc(-1), address(_address), val(0), pos(-1), offset(0) {
		rdset = string_to_reg(_rdset);
		reg_to_write.push_back(rdset);
		if (data_label.find(address) != data_label.end()) {
			pos = data_label[address];
			return;
		}
		int i = address.find('(');
		int j = address.find(')');
		offset = string_to_int(address.substr(0, i));
		rsrc = string_to_reg(address.substr(i + 1, j - i - 1));
		reg_to_read.push_back(rsrc);
	}
	virtual instruction* copy() { return new loading(*this); }
	virtual void data_prepare() { if (rsrc != -1) pos = reg[rsrc]; }
	virtual void execute() { pos += offset; }
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
	virtual void memory_access() { memcpy(&val, mem + pos, 1); }
};
class lh : public loading {
public:
	lh(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new lh(*this); }
	virtual void memory_access() { memcpy(&val, mem + pos, 2); }
};
class lw : public loading {
public:
	lw(const string &ph1, const string &ph2) : loading(ph1, ph2) {}
	virtual instruction* copy() { return new lw(*this); }
	virtual void memory_access() { memcpy(&val, mem + pos, 4); }
};

class storing : public instruction {
public:
	int rdset, rsrc;
	string address;
	int val, pos, offset;

	storing(const string &_rdset, const string &_address) :
		instruction(), rsrc(-1), address(_address), val(0), pos(-1), offset(0) {
		write_to_mem = true;
		rdset = string_to_reg(_rdset);
		reg_to_read.push_back(rdset);
		if (data_label.find(address) != data_label.end()) {
			pos = data_label[address];
			return;
		}
		int i = address.find('(');
		int j = address.find(')');
		offset = string_to_int(address.substr(0, i));
		rsrc = string_to_reg(address.substr(i + 1, j - i - 1));
		reg_to_read.push_back(rsrc);
	}
	virtual instruction* copy() { return new storing(*this); }
	virtual void data_prepare() {
		val = reg[rdset];
		if (rsrc != -1) pos = reg[rsrc];
	}
	virtual void execute() { pos += offset; }
};
class sb : public storing {
public:
	sb(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sb(*this); }
	virtual void memory_access() { 
		memcpy(mem + pos, &val, 1); 
		//cout << "sb: " << char(val) << " " << pos << endl;
	}
};
class sh : public storing {
public:
	sh(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sh(*this); }
	virtual void memory_access() { memcpy(mem + pos, &val, 2); }
};
class sw : public storing {
public:
	sw(const string &ph1, const string &ph2) : storing(ph1, ph2) {}
	virtual instruction* copy() { return new sw(*this); }
	virtual void memory_access() { 
		memcpy(mem + pos, &val, 4); 
		//cout << "sw: " << val << " " << pos << endl;
	}
};

class assignment : public instruction {
public:
	int rdset, rsrc, imm;

	assignment(const string &_rdset, const string &_rsrc) : instruction() {
		rdset = string_to_reg(_rdset);
		reg_to_write.push_back(rdset);
		rsrc = string_to_reg(_rsrc);
		if (rsrc == -1) imm = string_to_int(_rsrc);
		else reg_to_read.push_back(rsrc);
	}
	virtual instruction* copy() { return new assignment(*this); }
	virtual void data_prepare() { if (rsrc != -1) imm = reg[rsrc]; }
	virtual void write_back() { reg[rdset] = imm; }
};
class mfhi : public assignment {
public:
	mfhi(const string &_rdset) : assignment(_rdset, "$hi") {}
};
class mflo : public assignment {
public:
	mflo(const string &_rdset) : assignment(_rdset, "$lo") {}
};

class calculation : public instruction {
public:
	int rdset, rsrc1, rsrc2;
	int imm1, imm2, res;

	calculation(const string &_rdset, const string &_rsrc1, const string &_rsrc2) : instruction() {
		rdset = string_to_reg(_rdset);
		reg_to_write.push_back(rdset);
		rsrc1 = string_to_reg(_rsrc1);
		rsrc2 = string_to_reg(_rsrc2);
		if (rsrc1 == -1) imm1 = string_to_int(_rsrc1);
		else reg_to_read.push_back(rsrc1);
		if (rsrc2 == -1) imm2 = string_to_int(_rsrc2);
		else reg_to_read.push_back(rsrc2);
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
	virtual void execute() { res = imm1 - imm2; }
};
class mul : public calculation {
public:
	int para;
	bool unsign;
	long long llres;

	mul(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), para(ph3 == "" ? 2 : 3), unsign(_unsign) {
		if (para == 2) {
			reg_to_write.push_back(32);
			reg_to_write.push_back(33);
		}
	}
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
			reg[32] = llres; // lo
			reg[33] = (llres >> 32); // hi
		}
	}
};
class __div : public calculation { // & __divu
public:
	int para, q, r;
	bool unsign;

	__div(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), para(ph3 == "" ? 2 : 3), unsign(_unsign) {
		if (para == 2) {
			reg_to_write.push_back(32);
			reg_to_write.push_back(33);
		}
		//cout << "div: " << ph1 << " " << ph2 << " " << ph3 << " " << endl;
		//cout << reg_to_read.size(), 
	}
	virtual instruction* copy() { return new __div(*this); }
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
class __xor : public calculation { // & __xoru
public:
	bool unsign;

	__xor(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new __xor(*this); }
	virtual void execute() { res = imm1 ^ imm2; }
};
class neg : public calculation { // & negu
public:
	bool unsign;

	neg(const string &ph1, const string &ph2, bool _unsign) : calculation(ph1, ph2, ""), unsign(_unsign) {}
	virtual instruction* copy() { return new neg(*this); }
	virtual void execute() { res = -imm1; }
};
class rem : public calculation { // & remu
public:
	bool unsign;

	rem(const string &ph1, const string &ph2, const string &ph3, bool _unsign) : calculation(ph1, ph2, ph3), unsign(_unsign) {}
	virtual instruction* copy() { return new rem(*this); }
	virtual void execute() {
		if (unsign) res = (unsigned int)imm1 % (unsigned int)imm2;
		else res = imm1 % imm2;
		//cout << "rem: " << imm1 << " " << imm2 << endl;
	}
};
class seq : public calculation {
public:
	seq(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new seq(*this); }
	virtual void execute() { res = imm1 == imm2; }
};
class sge : public calculation {
public:
	sge(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new sge(*this); }
	virtual void execute() { res = imm1 >= imm2; }
};
class sgt : public calculation {
public:
	sgt(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new sgt(*this); }
	virtual void execute() { res = imm1 > imm2; }
};
class sle : public calculation {
public:
	sle(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new sle(*this); }
	virtual void execute() { res = imm1 <= imm2; }
};
class slt : public calculation {
public:
	slt(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new slt(*this); }
	virtual void execute() { res = imm1 < imm2; }
};
class sne : public calculation {
public:
	sne(const string &ph1, const string &ph2, const string &ph3) : calculation(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new sne(*this); }
	virtual void execute() { res = imm1 != imm2; }
};

class jump : public instruction { // b, j, jr
public:
	int rsrc, pos;

	jump(const string &label) : instruction(), rsrc(-1), pos(-1) {
		if (text_label.find(label) != text_label.end()) pos = text_label[label];
		else rsrc = string_to_reg(label), reg_to_read.push_back(rsrc);
		jump_type = 1;
	}
	virtual instruction* copy() { return new jump(*this); }
	virtual void data_prepare() { if (rsrc != -1) pos = reg[rsrc]; }
	virtual void write_back() {
		ins_top = pos;
	}
};
class jal : public instruction { // jal, jalr
public:
	int rsrc, pos;

	jal(const string &label) : instruction(), rsrc(-1), pos(-1) {
		if (text_label.find(label) != text_label.end()) pos = text_label[label];
		else rsrc = string_to_reg(label), reg_to_read.push_back(rsrc);
		jump_type = 1;
		reg_to_write.push_back(31);
	}
	virtual instruction* copy() { return new jal(*this); }
	virtual void data_prepare() { if (rsrc != -1) pos = reg[rsrc]; }
	virtual void write_back() {
		//cout << "ins_top: " << ins_top << endl;
		reg[31] = ins_top;
		ins_top = pos;
	}
};
class branch : public instruction {
public:
	int rsrc1, rsrc2;
	int imm1, imm2;
	int pos;
	bool judge;

	branch(const string &_rsrc1, const string &_rsrc2, const string &label) : instruction(), judge(false) {
		rsrc1 = string_to_reg(_rsrc1);
		rsrc2 = string_to_reg(_rsrc2);
		if (rsrc1 == -1) imm1 = string_to_int(_rsrc1);
		else reg_to_read.push_back(rsrc1);
		if (rsrc2 == -1) imm2 = string_to_int(_rsrc2);
		else reg_to_read.push_back(rsrc2);
		pos = text_label[label];
		jump_type = 2;
	}
	virtual instruction* copy() { return new branch(*this); }
	virtual void data_prepare() {
		if (rsrc1 != -1) imm1 = reg[rsrc1];
		if (rsrc2 != -1) imm2 = reg[rsrc2];
	}
	virtual void write_back() { if (judge) ins_top = pos; }
};
class beq : public branch {
public:
	beq(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new beq(*this); }
	virtual void execute() { judge = imm1 == imm2; }
};
class bne : public branch {
public:
	bne(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new bne(*this); }
	virtual void execute() { judge = imm1 != imm2; }
};
class bge : public branch {
public:
	bge(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new bge(*this); }
	virtual void execute() { judge = imm1 >= imm2; }
};
class ble : public branch {
public:
	ble(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new ble(*this); }
	virtual void execute() { judge = imm1 <= imm2; }
};
class bgt : public branch {
public:
	bgt(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new bgt(*this); }
	virtual void execute() { judge = imm1 > imm2; }
};
class blt : public branch {
public:
	blt(const string &ph1, const string  &ph2, const string &ph3) : branch(ph1, ph2, ph3) {}
	virtual instruction* copy() { return new blt(*this); }
	virtual void execute() { judge = imm1 < imm2; }
};

class syscall : public instruction {
public:
	istream &is;
	ostream &os;
	int type, val_a0, val_a1, res;
	//char str[MAXL];
	string str;

	syscall(istream &_is, ostream &_os) : instruction(), is(_is), os(_os) {
		//memset(str, 0, sizeof str);
		str = "";
		reg_to_read.push_back(2);
		reg_to_read.push_back(4);
		reg_to_read.push_back(5);
		reg_to_write.push_back(2);
		write_to_mem = true;
	}
	virtual instruction* copy() { return new syscall(*this); }
	virtual void data_prepare() {
		type = reg[2]; // $v0
		//cout << "syscall: " << type << endl;
		switch (type) {
		case 1: case 4: case 9: case 17: val_a0 = reg[4]; break; // $a0
		case 8:
			val_a0 = reg[4]; // $a0
			val_a1 = reg[5]; // $a1
			break;
		}
	}
	virtual void execute() {
		str = "";
		switch (type) {
		case 1: os << val_a0; break;
		case 5: is >> res; break;
		case 8: is >> str; break;
		case 10: shut_down(0); break;
		case 17: shut_down(val_a0); break;
		}
	}
	virtual void memory_access() {
		int i;
		switch (type) {
		case 4:
			i = val_a0;
			while (mem[i]) os << mem[i++];
			break;
		case 8:
			int l = str.length();// strlen(str);
			i = 0;
			while (i < l) mem[val_a0 + i] = str[i++];
			break;
		}
	}
	virtual void write_back() {
		switch (type) {
		case 5: reg[2] = res; break;
		case 9:
			reg[2] = heap_top;
			heap_top += val_a0;
			break;
		}
	}
};

void shut_down(int val) {
	for (int i = 0; i < 5; ++i)
		if (plat[i] != NULL && i > 0 && plat[i] != plat[i - 1]) delete plat[i];
	vector<instruction*>::iterator it = ins_vec.begin();
	while (it != ins_vec.end()) delete *(it++);
	while (true);
	exit(val);
}

// ============================= interpreter ==================================
class interpreter {
public:
	ifstream &src;
	istream &is;
	ostream &os;

	interpreter(ifstream &_src, istream &_is, ostream &_os) : src(_src), is(_is), os(_os) {
		memset(reg, 0, sizeof reg);
		memset(mem, 0, sizeof mem);
		reg[29] = MAXN - 1;
	} // $sp stack_top
	void interprete() {
		read_in();
		execute_text();
	}
	void read_in() {
		char str[MAXL];
		int ins_cnt = 0;
		vector<string> name_vec, ph1_vec, ph2_vec, ph3_vec;
		bool text_block = false;
		while (src.getline(str, MAXL, '\n')) {
			//cout << str << endl;
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
					bool flag = tmp == "asciiz";
					i += 2;
					tmp = get_str(str, i, l - 1);
					for (int i = 0; i < tmp.length(); ++i)
						mem[heap_top++] = tmp[i];
					if (flag) mem[heap_top++] = 0;
				}
				else if (tmp == "byte" || tmp == "half" || tmp == "word") {
					int m = tmp == "byte" ? 1 : (tmp == "half" ? 2 : 4);
					while (true) {
						if (i == l) break;
						++i;
						tmp = get_phrase(str, i, l);
						int n = string_to_int(tmp);
						memcpy(mem + heap_top, &n, m);
						heap_top += m;
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
				if (text_block) text_label[tmp] = ins_cnt;
				else data_label[tmp] = heap_top;
			}
			else { // text instruction
				string name = get_phrase(str, i, l); ++i;
				if (name == "") continue;
				++ins_cnt;
				name_vec.push_back(name);
				ph1_vec.push_back(get_phrase(str, i, l)); ++i;
				ph2_vec.push_back(get_phrase(str, i, l)); ++i;
				ph3_vec.push_back(get_phrase(str, i, l)); ++i;
			}
		}
		//cout << "ins_cnt: " << ins_cnt << endl;
		for (int i = 0; i < ins_cnt; ++i) {
			string name = name_vec[i];
			string ph1 = ph1_vec[i], ph2 = ph2_vec[i], ph3 = ph3_vec[i];
			instruction *ptr = NULL;
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
			if (name == "add") ptr = new add(ph1, ph2, ph3, false);
			if (name == "addu" || name == "addiu") ptr = new add(ph1, ph2, ph3, true);
			if (name == "sub") ptr = new sub(ph1, ph2, ph3, false);
			if (name == "subu") ptr = new sub(ph1, ph2, ph3, true);
			if (name == "mul") ptr = new mul(ph1, ph2, ph3, false);
			if (name == "mulu") ptr = new mul(ph1, ph2, ph3, true);
			if (name == "div") ptr = new __div(ph1, ph2, ph3, false);
			if (name == "divu") ptr = new __div(ph1, ph2, ph3, true);
			if (name == "xor") ptr = new __xor(ph1, ph2, ph3, false);
			if (name == "xoru") ptr = new __xor(ph1, ph2, ph3, true);
			if (name == "neg") ptr = new neg(ph1, ph2, false);
			if (name == "negu") ptr = new neg(ph1, ph2, true);
			if (name == "rem") ptr = new rem(ph1, ph2, ph3, false);
			if (name == "remu") ptr = new rem(ph1, ph2, ph3, true);
			if (name == "seq") ptr = new seq(ph1, ph2, ph3);
			if (name == "sne") ptr = new sne(ph1, ph2, ph3);
			if (name == "sge") ptr = new sge(ph1, ph2, ph3);
			if (name == "sle") ptr = new sle(ph1, ph2, ph3);
			if (name == "sgt") ptr = new sgt(ph1, ph2, ph3);
			if (name == "slt") ptr = new slt(ph1, ph2, ph3);
			// jump instruction
			if (name == "b" || name == "j" || name == "jr") ptr = new jump(ph1);
			if (name == "jal" || name == "jalr") ptr = new jal(ph1);
			if (name == "beq") ptr = new beq(ph1, ph2, ph3);
			if (name == "bne") ptr = new bne(ph1, ph2, ph3);
			if (name == "bge") ptr = new bge(ph1, ph2, ph3);
			if (name == "ble") ptr = new ble(ph1, ph2, ph3);
			if (name == "bgt") ptr = new bgt(ph1, ph2, ph3);
			if (name == "blt") ptr = new blt(ph1, ph2, ph3);
			if (name == "beqz") ptr = new beq(ph1, "0", ph2);
			if (name == "bnez") ptr = new bne(ph1, "0", ph2);
			if (name == "bgez") ptr = new bge(ph1, "0", ph2);
			if (name == "blez") ptr = new ble(ph1, "0", ph2);
			if (name == "bgtz") ptr = new bgt(ph1, "0", ph2);
			if (name == "bltz") ptr = new blt(ph1, "0", ph2);
			// syscall instruction
			if (name == "syscall") ptr = new syscall(is, os);
			if (name == "nop") ptr = new instruction();
			ins_vec.push_back(ptr);
			
			//cout << i << ": " << name << " " << ph1 << " " << ph2 << " " << ph3 << " : \n";

			/*
			auto it = ptr->reg_to_read.begin();
			while (it != ptr->reg_to_read.end()) cout << *(it++) << " ";
			cout << endl;
			it = ptr->reg_to_write.begin();
			while (it != ptr->reg_to_write.end()) cout << *(it++) << " ";
			cout << endl;
			*/
		}
	}
	void execute_text() {
		ins_top = text_label["main"];
		int ins_vec_sz = ins_vec.size();
		ull reg_to_write = 0;
		int jump_cnt = 0;
		int reg_cnt[34], cnt = 0;
		memset(reg_cnt, 0, sizeof reg_cnt);
		memset(plat, NULL, sizeof plat);
		vector<int>::iterator it;
		while (true) {

			//cout << "$t1: " << reg[9] << endl;
			//cout << "$a0: " << reg[4] << endl;

			if(true) {
			//cout << ins_top << endl;
			instruction *ptr = ins_vec[ins_top++];
			ptr->data_prepare();
			ptr->execute();
			ptr->memory_access();
			ptr->write_back();
			}
			else {

				//if (ins_top > 527) cout << "haha\n";
			
			// write back
			if (plat[4] != NULL) {
				plat[4]->write_back();
				jump_cnt -= plat[4]->jump_type;
				it = plat[4]->reg_to_write.begin();
				while (it != plat[4]->reg_to_write.end()) --reg_cnt[*(it++)];
				delete plat[4];
				plat[4] = NULL;
				--cnt;
			}
			// memory access
			bool write_to_mem = false;
			if (plat[3] != NULL) {
				plat[3]->memory_access();
				write_to_mem = plat[3]->write_to_mem;
			}
			plat[4] = plat[3];
			// execute
			if (plat[2] != NULL) plat[2]->execute();
			plat[3] = plat[2];
			// data prepare
			if (plat[1] != NULL) plat[1]->data_prepare();
			plat[2] = plat[1];
			// instruction fetch
			plat[1] = plat[0];
			plat[0] = NULL;
			if (ins_top >= ins_vec_sz) continue;
			instruction *ptr = ins_vec[ins_top]->copy();
			bool reg_conflict = false;
			it = ptr->reg_to_read.begin();
			while (it != ptr->reg_to_read.end())
				if (reg_cnt[*(it++)] > 0) reg_conflict = true;
			if (jump_cnt > 0 || write_to_mem || reg_conflict) delete ptr, ptr = NULL;
			else {
				jump_cnt += ptr->jump_type;
				it = ptr->reg_to_write.begin();
				while (it != ptr->reg_to_write.end()) ++reg_cnt[*(it++)];
				//cout << ins_top << endl;
				++ins_top;
				++cnt;
			}
			plat[0] = ptr;
			}
		}
	}
};

int main(int argc, char *argv[]) {
//int main() {

	//cout << "?????\n";

	ifstream source;
	ifstream input;

	source.open(argv[1]);
	//source.open("gcd-5090379042-jiaxiao.s");
	//input.open("gcd-5090379042-jiaxiao.in");

	interpreter itp(source, cin, cout);
	//interpreter itp(source, input, cout);
	itp.interprete();
	shut_down(0);

	return 0;
}