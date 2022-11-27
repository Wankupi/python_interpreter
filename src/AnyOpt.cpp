#include "AnyOpt.h"
#include "func.h"
#include <cmath>

void testVar(antlrcpp::Any &any) {
	if (any.is<VarType>()) any = any.as<VarType>()->second;
}

Int toInt(antlrcpp::Any const &a) {
	if (a.is<Int>())
		return a.as<Int>();
	else if (a.is<double>())
		return Int{a.as<double>()};
	else if (a.is<string>())
		return Int(a.as<string>());
	else if (a.is<bool>())
		return a.as<bool>() ? 1 : 0;
	else
		return 0;
}

double toFloat(antlrcpp::Any const &a) {
	if (a.is<double>())
		return a.as<double>();
	else if (a.is<Int>())
		return a.as<Int>().to_double();
	else if (a.is<string>())
		return std::stod(a.as<string>());
	else if (a.is<bool>())
		return a.as<bool>() ? 1.0 : 0.0;
	else
		return 0.0;
}

bool toBool(antlrcpp::Any const &a) {
	if (a.is<bool>())
		return a.as<bool>();
	else if (a.is<Int>())
		return bool(a.as<Int>());
	else if (a.is<double>())
		return bool(a.as<double>());
	else if (a.is<string>())
		return a.as<string>().length() > 0;
	else
		return false;
}

std::string toStr(antlrcpp::Any const &a) {
	if (a.is<string>())
		return a;
	else if (a.is<Int>())
		return a.as<Int>().to_string();
	else if (a.is<double>())
		return std::to_string(a.as<double>());
	else if (a.is<bool>())
		return a.as<bool>() ? "True" : "False";
	else
		return string{};
}

antlrcpp::Any operator+(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.is<string>() || b.is<string>())
		return toStr(a) + toStr(b);
	else if (a.is<double>() || b.is<double>())
		return toFloat(a) + toFloat(b);
	else
		return toInt(a) + toInt(b);
}

antlrcpp::Any operator-(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.is<double>() || b.is<double>())
		return toFloat(a) - toFloat(b);
	else
		return toInt(a) - toInt(b);
}

antlrcpp::Any operator*(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.is<string>() || b.is<string>()) {
		string s = a.is<string>() ? a.as<string>() : b.as<string>();
		Int times = toInt(a.is<string>() ? b : a);
		return s * int(times);
	}
	else if (a.is<double>() || b.is<double>()) {
		double x = toFloat(a), y = toFloat(b);
		return x * y;
	}
	else {
		Int x = toInt(a), y = toInt(b);
		return x * y;
	}
}

antlrcpp::Any DivInt(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.is<double>() || b.is<double>()) {
		double x = toFloat(a), y = toFloat(b);
		return floor(x / y);
	}
	else {
		return toInt(a) / toInt(b);
	}
}

antlrcpp::Any DivFloat(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	double x = toFloat(a), y = toFloat(b);
	return x / y;
}

antlrcpp::Any operator%(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	return a - DivInt(a, b) * b;
}

antlrcpp::Any &operator+=(antlrcpp::Any &a, antlrcpp::Any const &b) {
	if (a.is<Int>() && b.is<Int>()) {
		a.as<Int>() += b.as<Int>();
		return a;
	}
	return a = a + b;
}

antlrcpp::Any &operator-=(antlrcpp::Any &a, antlrcpp::Any const &b) {
	if (a.is<Int>() && b.is<Int>()) {
		a.as<Int>() -= b.as<Int>();
		return a;
	}
	return a = a - b;
}

antlrcpp::Any &operator*=(antlrcpp::Any &a, antlrcpp::Any const &b) {
	if (a.is<Int>() && b.is<Int>()) {
		a.as<Int>() *= b.as<Int>();
		return a;
	}
	else return a = a * b;
}

antlrcpp::Any &operator%=(antlrcpp::Any &a, antlrcpp::Any const &b) {
	a -= DivInt(a, b) * b;
	return a;
}

antlrcpp::Any operator-(antlrcpp::Any const &a) {
	if (a.is<double>())
		return -a.as<double>();
	else
		return -toInt(a);
}

void setAnyNegative(antlrcpp::Any &a) {
	if (a.is<double>())
		a.as<double>() = -a.as<double>();
	else {
		if (!a.is<Int>()) a = toInt(a);
		a.as<Int>().toggleSign();
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
bool operator<(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.is<string>()) return b.is<string>() && a.as<string>() < b.as<string>();
	else if (a.is<double>() || b.is<double>()) return toFloat(a) < toFloat(b);
	else if (a.is<Int>() || b.is<Int>()) return toInt(a) < toInt(b);
	else return toBool(a) < toBool(b);
}

bool operator>(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	return b < a;
}

/**
 * @attention None only equal to None
 * @attention str only equal to str
 */
bool operator==(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	if (a.isNull() || b.isNull()) return a.isNull() && b.isNull();
	if (a.is<string>() || b.is<string>()) return a.is<string>() && b.is<string>() && a.as<string>() == b.as<string>();
	else if (a.is<double>() || b.is<double>()) return toFloat(a) == toFloat(b);
	else if (a.is<Int>() || b.is<Int>()) return toInt(a) == toInt(b);
	else return toBool(a) == toBool(b);
}

bool operator>=(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	return !(a < b);
}

bool operator<=(antlrcpp::Any const &a, antlrcpp::Any const &b) {
	return !(b < a);
}

bool operator!=(antlrcpp::Any const &a, antlrcpp::Any const &b) {
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

std::ostream &operator<<(std::ostream &os, antlrcpp::Any const &a) {
	if (a.is<VarType>())
		os << a.as<VarType>()->second;
	else if (a.is<string>())
		os << a.as<string>();
	else if (a.is<Int>())
		os << a.as<Int>();
	else if (a.is<double>()) {
		double val = a.as<double>();
		constexpr double eps = 1e-9;
		if (val > -eps && val < eps) val = 0;
		os << std::fixed << std::setprecision(6) << val;
	}
	else if (a.is<bool>())
		os << (a.as<bool>() ? "True" : "False");
	else if (a.isNull())
		os << "None";
	else if (a.is<std::shared_ptr<func>>())
		os << "function<" << a.as<std::shared_ptr<func>>()->getName() << ">";
	return os;
}
