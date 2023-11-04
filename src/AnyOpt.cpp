#include "AnyOpt.h"
#include "func.h"
#include <cmath>
#include <iomanip>

void testVar(std::any &any) {
	if (is<std::vector<std::any>>(any)) {
		std::vector<std::any> &v = as<std::vector<std::any>>(any);
		for (auto &x : v)
			testVar(x);
	}
	if (is<VarType>(any)) any = as<VarType>(any)->second;
}

Int toInt(std::any const &a) {
	if (is<Int>(a))
		return as<Int>(a);
	else if (is<double>(a))
		return Int{as<double>(a)};
	else if (is<string>(a))
		return Int(as<string>(a));
	else if (is<bool>(a))
		return as<bool>(a) ? 1 : 0;
	else
		return 0;
}

double toFloat(std::any const &a) {
	if (is<double>(a))
		return as<double>(a);
	else if (is<Int>(a))
		return as<Int>(a).to_double();
	else if (is<string>(a))
		return std::stod(as<string>(a));
	else if (is<bool>(a))
		return as<bool>(a) ? 1.0 : 0.0;
	else
		return 0.0;
}

bool toBool(std::any const &a) {
	if (is<bool>(a))
		return as<bool>(a);
	else if (is<Int>(a))
		return bool(as<Int>(a));
	else if (is<double>(a))
		return bool(as<double>(a));
	else if (is<string>(a))
		return as<string>(a).length() > 0;
	else
		return false;
}

std::string toStr(std::any const &a) {
	if (is<string>(a))
		return as<string>(a);
	else if (is<Int>(a))
		return as<Int>(a).to_string();
	else if (is<double>(a))
		return std::to_string(as<double>(a));
	else if (is<bool>(a))
		return as<bool>(a) ? "True" : "False";
	else
		return "None";
}

std::any operator+(std::any const &a, std::any const &b) {
	if (is<string>(a) || is<string>(b))
		return toStr(a) + toStr(b);
	else if (is<double>(a) || is<double>(b))
		return toFloat(a) + toFloat(b);
	else
		return toInt(a) + toInt(b);
}

std::any operator-(std::any const &a, std::any const &b) {
	if (is<double>(a) || is<double>(b))
		return toFloat(a) - toFloat(b);
	else
		return toInt(a) - toInt(b);
}

std::any operator*(std::any const &a, std::any const &b) {
	if (is<string>(a) || is<string>(b)) {
		string s = is<string>(a) ? as<string>(a) : as<string>(b);
		Int times = toInt(is<string>(a) ? b : a);
		return s * int(times);
	}
	else if (is<double>(a) || is<double>(b)) {
		double x = toFloat(a), y = toFloat(b);
		return x * y;
	}
	else {
		Int x = toInt(a), y = toInt(b);
		return x * y;
	}
}

std::any DivInt(std::any const &a, std::any const &b) {
	if (is<double>(a) || is<double>(b)) {
		double x = toFloat(a), y = toFloat(b);
		return floor(x / y);
	}
	else {
		return toInt(a) / toInt(b);
	}
}

std::any DivFloat(std::any const &a, std::any const &b) {
	double x = toFloat(a), y = toFloat(b);
	return x / y;
}

std::any operator%(std::any const &a, std::any const &b) {
	return a - DivInt(a, b) * b;
}

std::any &operator+=(std::any &a, std::any const &b) {
	if (is<Int>(a) && is<Int>(b)) {
		as<Int>(a) += as<Int>(b);
		return a;
	}
	return a = a + b;
}

std::any &operator-=(std::any &a, std::any const &b) {
	if (is<Int>(a) && is<Int>(b)) {
		as<Int>(a) -= as<Int>(b);
		return a;
	}
	return a = a - b;
}

std::any &operator*=(std::any &a, std::any const &b) {
	if (is<Int>(a) && is<Int>(b)) {
		as<Int>(a) *= as<Int>(b);
		return a;
	}
	else return a = a * b;
}

std::any &operator%=(std::any &a, std::any const &b) {
	a -= DivInt(a, b) * b;
	return a;
}

std::any operator-(std::any const &a) {
	if (is<double>(a))
		return -as<double>(a);
	else
		return -toInt(a);
}

void setAnyNegative(std::any &a) {
	if (is<double>(a))
		as<double>(a) = -as<double>(a);
	else {
		if (!is<Int>(a)) a = toInt(a);
		as<Int>(a).toggleSign();
	}
}

/**
 * @attention no need consider none, str < non-str
 * @details possible compare type
 * @details str \< str,
 * @details int \< float, float \< int, float \< float, float \< bool,
 * @details int \< int, int \< bool,
 * @details bool \< bool
 */
bool operator<(std::any const &a, std::any const &b) {
	if (is<string>(a)) return is<string>(b) && as<string>(a) < as<string>(b);
	else if (is<double>(a) || is<double>(b)) return toFloat(a) < toFloat(b);
	else if (is<Int>(a) || is<Int>(b)) return toInt(a) < toInt(b);
	else return toBool(a) < toBool(b);
}

bool operator>(std::any const &a, std::any const &b) {
	return b < a;
}

/**
 * @attention None only equal to None
 * @attention str only equal to str
 */
bool operator==(std::any const &a, std::any const &b) {
	if (isNull(a) || isNull(b)) return isNull(a) && isNull(b);
	if (is<string>(a) || is<string>(b)) return is<string>(a) && is<string>(b) && as<string>(a) == as<string>(b);
	else if (is<double>(a) || is<double>(b)) return toFloat(a) == toFloat(b);
	else if (is<Int>(a) || is<Int>(b)) return toInt(a) == toInt(b);
	else return toBool(a) == toBool(b);
}

bool operator>=(std::any const &a, std::any const &b) {
	return !(a < b);
}

bool operator<=(std::any const &a, std::any const &b) {
	return !(b < a);
}

bool operator!=(std::any const &a, std::any const &b) {
	return !(a == b);
}

// string fast multiply
string operator*(string x, int b) {
	string r;
	r.reserve(x.length() * b);
	while (b) {
		if (b & 1) r += x;
		b >>= 1;
		if (b) x += x;
	}
	return r;
}

std::ostream &operator<<(std::ostream &os, std::any const &a) {
	if (is<VarType>(a))
		os << as<VarType>(a)->second;
	else if (is<string>(a))
		os << as<string>(a);
	else if (is<Int>(a))
		os << as<Int>(a);
	else if (is<double>(a)) {
		double val = as<double>(a);
		constexpr double eps = 1e-9;
		if (val > -eps && val < eps) val = 0;
		os << std::fixed << std::setprecision(6) << val;
	}
	else if (is<bool>(a))
		os << (as<bool>(a) ? "True" : "False");
	else if (isNull(a))
		os << "None";
	else if (is<std::shared_ptr<func>>(a))
		os << "function<" << as<std::shared_ptr<func>>(a)->getName() << ">";
	else if (is<std::vector<std::any>>(a)) {
		std::vector<std::any> const &v = as<std::vector<std::any>>(a);
		if (v.empty()) return os << "()";
		os << "(" << v[0];
		for (size_t i = 1; i < v.size(); ++i)
			os << ", " << v[i];
		os << ")";
	}
	return os;
}
