#include "tool.h"

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

string get_phrase(char * str, int & i, int l) {
	string res = "";
	while (i < l && str[i] != ' ') res += str[i++];
	return res;
}

string get_str(char * str, int & i, int l) {
	string res = "";
	while (i < l) {
		res += str[i++];
		if (res.back() == '\\') {
			char ch = str[i++], real = '\0';
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
