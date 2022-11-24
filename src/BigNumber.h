#pragma once
#ifndef KUPI_INT_HEADER
#define KUPI_INT_HEADER

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

class number_basis {
public:
	virtual std::string to_string() const = 0;
	explicit operator std::string() const { return to_string(); }
	int len() const { return data.size(); }
	ValueType &operator[](int x) { return data[x]; }
	ValueType const &operator[](int x) const { return data[x]; }

protected:
	number_basis() : neg(false), data() {}
	number_basis(bool _neg, std::vector<ValueType> data) : neg(_neg), data(std::move(data)) {}
	number_basis(number_basis const &) = default;
	number_basis(number_basis &&x) : neg(x.neg), data(std::move(x.data)) {}
	number_basis &operator=(number_basis const &x) = default;
	number_basis &operator=(number_basis &&x) {
		neg = x.neg;
		data.swap(x.data);
		return *this;
	}

	void remove_front_zero();

public:
	bool neg;

protected:
	std::vector<ValueType> data;
};

class Int : public number_basis {
public:
	Int(int x) : Int((long long)(x)) {}
	Int(double x) : Int((long long)(x)) {}
	Int(long long = 0);
	Int(__int128);
	explicit Int(std::string const &);
	Int(Int const &x) = default;
	Int(Int &&x) noexcept : number_basis(std::forward<Int>(x)) {}
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
	std::string to_string() const override;
	double to_double() const { return std::stod(this->to_string()); }

	// friend std::istream &operator>>(std::istream &, Int &);
	friend std::ostream &operator<<(std::ostream &, const Int &);

private:
	Int(bool _neg, std::vector<ValueType> _a) : number_basis(_neg, std::move(_a)) {}
	__int128 getFrontBits(int k) const;
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
