#pragma once
#ifndef KUPI_INT_HEADER
#define KUPI_INT_HEADER

#include <iostream>
#include <string>
#include <vector>

namespace kupi {

using ValueType = int;
constexpr ValueType pow_10(int a) {
	ValueType r = 1;
	while (a) {
		r *= 10;
		--a;
	}
	return r;
}
constexpr int pressBit = 2;
constexpr ValueType P = pow_10(pressBit);

class Int {
public:
	Int(int x) : Int((long long)(x)) {}
	Int(double x) : Int(std::to_string(x)) {}
	Int(long long x) : Int(__int128(x)) {}
	Int(__int128 = 0);
	Int(std::string const &);
	Int(Int const &x) = default;
	Int(Int &&x) noexcept : neg(x.neg), data(std::move(x.data)) {}
	Int &operator=(Int const &x) = default;
	Int &operator=(Int &&x) {
		neg = x.neg;
		data.swap(x.data);
		return *this;
	}
	void toggleSign() {
		neg = !neg;
		if (data.size() == 1 && data[0] == 0) neg = false;
	}
	Int operator-() const;
	Int operator+() const;
	Int &operator<<=(int x);
	Int &operator>>=(int x);
	Int &operator+=(Int const &);
	Int &operator-=(Int const &);
	Int &operator*=(Int const &);
	Int &operator/=(Int const &);
	friend Int operator+(const Int &, const Int &);
	friend Int operator-(const Int &, const Int &);
	friend Int operator*(const Int &, const Int &);
	friend Int operator/(const Int &, const Int &);
	explicit operator int() const;
	explicit operator bool() const;
	explicit operator double() const { return to_double(); }
	explicit operator std::string() const { return to_string(); }
	std::string to_string() const;
	double to_double() const { return std::stod(this->to_string()); }
	void read(std::string const &);
	void print() const;
	friend std::istream &operator>>(std::istream &, Int &);
	friend std::ostream &operator<<(std::ostream &, const Int &);
	int len() const { return static_cast<int>(data.size()); }
	ValueType &operator[](int x) { return data[x]; }
	ValueType const &operator[](int x) const { return data[x]; }

private:
	Int(bool neg, std::vector<ValueType> data) : neg(neg), data(std::move(data)) {}
	__int128 getFrontBits(int k) const;
	void remove_front_zero();

public:
	bool neg = false;

private:
	std::vector<ValueType> data;
};

bool cmpAbsLess(Int const &, Int const &);
bool cmpAbsLessEq(Int const &, Int const &);
bool cmpAbsEq(Int const &, Int const &);

bool operator==(const Int &, const Int &);
bool operator!=(const Int &, const Int &);
bool operator<(const Int &, const Int &);
bool operator>(const Int &, const Int &);
bool operator<=(const Int &, const Int &);
bool operator>=(const Int &, const Int &);

} // namespace kupi

using Int = kupi::Int;

#endif
