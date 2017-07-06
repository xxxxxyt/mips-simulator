#ifndef _instruction_hpp
#define _instruction_hpp

#include <vector>
#include <cstdio>
#include <string>
#include <map>
#include <cstring>
#include "state.h"
#include "tool.h"
using namespace std;

extern void shut_down(int val);

class instruction {
public:
	vector<int> reg_to_read, reg_to_write;
	int line, jump_type; // 0 not jump; 1 jump; 2 branch

	instruction() : jump_type(0) {
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
	virtual void memory_access() { memcpy(mem + pos, &val, 1); }
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
	virtual void memory_access() { memcpy(mem + pos, &val, 4); }
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
			reg_to_read.push_back(rdset);
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
			reg_to_read.push_back(rdset);
			reg_to_write.push_back(32);
			reg_to_write.push_back(33);
		}
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
	virtual void write_back() { ins_top = pos; }
};
class jal : public instruction { // jal, jalr
public:
	int rsrc, pos, val;

	jal(const string &label) : instruction(), rsrc(-1), pos(-1), val(0) {
		if (text_label.find(label) != text_label.end()) pos = text_label[label];
		else rsrc = string_to_reg(label), reg_to_read.push_back(rsrc);
		jump_type = 1;
		reg_to_write.push_back(31);
	}
	virtual instruction* copy() { return new jal(*this); }
	virtual void data_prepare() { if (rsrc != -1) pos = reg[rsrc]; }
	virtual void write_back() { 
		reg[31] = ins_top; 
		ins_top = pos;
	}
};
class branch : public instruction {
public:
	int rsrc1, rsrc2;
	int imm1, imm2;
	int pos;
	bool judge, predict;

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
	string str;

	syscall(istream &_is, ostream &_os) : instruction(), is(_is), os(_os) {
		str = "";
		reg_to_read.push_back(2);
		reg_to_read.push_back(4);
		reg_to_read.push_back(5);
		reg_to_write.push_back(2);
	}
	virtual instruction* copy() { return new syscall(*this); }
	virtual void data_prepare() {
		type = reg[2]; // $v0
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
			int l = str.length();
			i = 0;
			while (i < l) mem[val_a0 + i] = str[i], ++i;
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

#endif