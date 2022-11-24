#include "BigNumber.h"
#include <algorithm>
#include <iostream>
#include <map>

namespace kupi {

struct FFT {
public:
	static constexpr int mod = 998244353;

	static int pow(int x, int b) {
		int r = 1;
		while (b) {
			if (b & 1) r = (long long)r * x % mod;
			x = (long long)x * x % mod;
			b >>= 1;
		}
		return r;
	}

	static int W(int k, int n, int type) { return pow(type == 1 ? 3 : 332748118, (mod - 1) / n * k); }

	static int pls(int a, int b) { return a + b < mod ? a + b : a + b - mod; }

	static int dec(int a, int b) { return a < b ? a - b + mod : a - b; }

	explicit FFT(int n) : limit(1) {
		int m = 0;
		while (limit <= n) {
			++m;
			limit <<= 1;
		}
		if (revs.find(limit) == revs.end()) {
			std::vector<int> &rev = revs[limit];
			rev.resize(limit);
			for (int i = 1; i < limit; ++i)
				rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (m - 1));
		}
	}

	void DFT(std::vector<int> &f, int op) {
		std::vector<int> const &rev = revs[limit];
		for (int i = 0; i != limit; ++i)
			if (rev[i] < i) std::swap(f[i], f[rev[i]]);
		for (int len = 1; len < limit; len <<= 1) {
			int W1 = W(1, 2 * len, op);
			for (int l = 0; l < limit; l += len * 2) {
				int ome = 1;
				for (int i = 0; i < len; ++i) {
					int x = f[l + i], y = f[l + len + i];
					f[l + i] = pls(x, (long long)ome * y % mod);
					f[l + len + i] = dec(x, (long long)ome * y % mod);
					ome = (long long)ome * W1 % mod;
				}
			}
		}
		if (op == -1) {
			long long inv = pow(limit, mod - 2);
			for (int i = 0; i < limit; ++i)
				f[i] = f[i] * inv % mod;
		}
	}

public:
	int limit;
	static std::map<int, std::vector<int>> revs;
};
std::map<int, std::vector<int>> FFT::revs;

void number_basis::remove_front_zero() {
	while (data.back() == 0 && data.size() > 1)
		data.pop_back();
	if (data.size() == 1 && data[0] == 0) neg = false;
}

Int::Int(long long x) {
	if (x < 0) {
		neg = true;
		x = -x;
	}
	int bit = 0;
	long long _x = x;
	while (_x) {
		++bit;
		_x /= P;
	}
	if (bit == 0)
		data = {0};
	else {
		data.reserve(bit);
		while (x) {
			data.emplace_back(x % P);
			x /= P;
		}
	}
}

Int::Int(__int128 x) {
	if (x < 0) {
		neg = true;
		x = -x;
	}
	int bit = 0;
	__int128 _x = x;
	while (_x) {
		++bit;
		_x /= P;
	}
	if (bit == 0)
		data = {0};
	else {
		data.reserve(bit);
		while (x) {
			data.emplace_back(x % P);
			x /= P;
		}
	}
}

Int::Int(std::string const &str) {
	data.reserve((str.length() + pressBit - 1) / pressBit);
	ValueType val = 0;
	int haveBits = 0;
	for (auto cur = str.crbegin(); cur != str.crend(); ++cur) {
		if (!isdigit(*cur)) {
			if (*cur == '-') neg = true;
			break;
		}
		val = val + pow_10(haveBits) * (*cur - '0');
		++haveBits;
		if (haveBits == pressBit) {
			data.emplace_back(val);
			val = 0;
			haveBits = 0;
		}
	}
	if (haveBits > 0)
		data.emplace_back(val);
	if (data.empty()) data.emplace_back(0);
	remove_front_zero();
}

Int Int::operator-() const { return Int(!neg, data); }

Int Int::operator+() const { return *this; }

Int &Int::operator+=(Int const &b) {
	if (neg == b.neg) {
		int lenA = len(), lenB = b.len();
		int mxlen = std::max(lenA, lenB) + 1;
		data.resize(mxlen);
		for (int i = 0; i < b.len(); ++i)
			data[i] += b[i];
		bool up = false;
		for (int i = 0; i < mxlen; ++i) {
			if (up) ++data[i];
			if (data[i] >= P) {
				data[i] -= P;
				up = true;
			}
			else
				up = false;
		}
	}
	else {
		neg = !neg;
		*this -= b;
		neg = !neg;
	}
	remove_front_zero();
	return *this;
}

Int &Int::operator-=(Int const &b) {
	if (neg == b.neg) {
		int lenA = len(), lenB = b.len();
		int mxlen = std::max(lenA, lenB) + 1;
		data.resize(mxlen);
		for (int i = 0; i < b.len(); ++i)
			data[i] -= b[i];
		remove_front_zero();
		mxlen = data.size();

		bool more_than_zero = data[mxlen - 1] > 0;
		if (!more_than_zero) {
			neg = !neg;
			for (int i = 0; i < mxlen; ++i)
				data[i] = -data[i];
		}

		bool down = false;
		for (int i = 0; i < mxlen; ++i) {
			if (down) --data[i];
			if (data[i] < 0) {
				down = true;
				data[i] += P;
			}
			else
				down = false;
		}
	}
	else {
		neg = !neg;
		*this += b;
		neg = !neg;
	}
	remove_front_zero();
	return *this;
}

Int &Int::operator*=(Int const &b) {
	int N = len() + b.len();
	FFT r(N);
	std::vector<int> &fa = data, fb(b.data);
	fa.resize(r.limit, 0);
	fb.resize(r.limit, 0);
	r.DFT(fa, +1);
	r.DFT(fb, +1);
	for (int i = 0; i < r.limit; ++i)
		fa[i] = (long long)(fa[i]) * fb[i] % FFT::mod;
	r.DFT(fa, -1);
	int up = 0;
	for (int i = 0; i < r.limit; ++i) {
		fa[i] += up;
		up = fa[i] / P;
		fa[i] %= P;
	}
	neg = (neg != b.neg);
	remove_front_zero();
	return *this;
}

Int &Int::operator/=(Int const &b) {
	return *this = *this / b;
}

Int operator+(const Int &a, const Int &b) {
	return Int(a) += b;
}

Int operator-(const Int &a, const Int &b) {
	return Int(a) -= b;
}

Int operator*(const Int &a, const Int &b) {
	return Int(a) *= b;
}

class Float : public Int {
public:
	explicit Float(Int x) : Int(std::move(x)), k(0) {}
	explicit Float(double x);
	Float(Float const &) = default;
	Float(Float &&) = default;
	Float &operator<<=(int t) {
		k += t;
		return *this;
	}

	Float &operator>>=(int t) {
		k -= t;
		return *this;
	}
	Float &operator*=(Float const &b);
	friend Float operator*(Float const &a, Float const &b);

	void cut(int bit) {
		int toCut = data.size() - bit;
		if (toCut <= 0) return;
		k += toCut;
		data.erase(data.begin(), data.begin() + std::min(toCut, int(data.size())));
	}

	Int &toInt() {
		cut(int(data.size()) + k);
		if (data.empty()) data = {0};
		return *this;
	}

	Float &minus_2_float();

	friend Float getInverse(Int const &, int);

private:
	int k;
};

Float &Float::operator*=(Float const &b) {
	dynamic_cast<Int &>(*this) *= dynamic_cast<Int const &>(b);
	k += b.k;
	return *this;
}

Float operator*(Float const &a, Float const &b) {
	return Float(a) *= b;
}

// not concern sign
Float &Float::minus_2_float() {
	int n = -k + 1;
	if (int(data.size()) < n)
		data.resize(n);

	bool down = false;
	for (int i = 0; i < n; ++i) {
		if (i == n - 1)
			data[i] = 2 - data[i] - int(down);
		else {
			data[i] = -data[i] - int(down);
			if (data[i] < 0) {
				data[i] += P;
				down = true;
			}
			else
				down = false;
		}
	}
	return *this;
}

Float getInverse(Int const &_a, int bit_need) {
	Float a(_a);
	a >>= a.len();
	Float x(Int(P / (a[a.len() - 1] + 1)));
	auto do_once = [&x, &a](int bit) {
		Float &&r = a * x;
		r.cut(bit);
		x *= r.minus_2_float();
		x.cut(bit);
	};
	for (int k = 1; k <= 4; ++k)
		do_once(4);
	for (int s = 4; s <= bit_need; s <<= 1)
		do_once(s * 2);
	x.cut(bit_need);
	x >>= a.len();
	x.neg = _a.neg;
	return x;
}

Int operator/(const Int &a, const Int &b) {
	if (a.len() <= 17 && b.len() <= 17) {
		__int128 v1 = a.getFrontBits(17), v2 = b.getFrontBits(17);
		if (a.neg == b.neg) return v1 / v2;
		return -(v1 + v2 - 1) / v2;
	}
	Float invb = getInverse(b, std::max(a.len(), b.len()));
	Float r = Float(a) * invb;
	Int &ret = r.toInt();
	ret.neg = false;
	Int nowLast = ret * b;
	nowLast.neg = a.neg;
	nowLast = a - nowLast;
	nowLast.remove_front_zero();
	nowLast.neg = b.neg;
	while (cmpAbsLessEq(b, nowLast)) {
		nowLast -= b;
		ret += 1;
	}
	if (a.neg != b.neg && !cmpAbsEq(nowLast, 0)) ret += 1;
	ret.neg = (a.neg != b.neg);
	return ret;
}

Int::operator int() const {
	int muti = 1, ret = 0;
	for (ValueType v : data) {
		ret += v * muti;
		muti *= P;
	}
	return ret;
}

Int::operator bool() const {
	return data.size() != 1 || data[0] != 0;
}

std::string Int::to_string() const {
	std::string buff;
	buff.reserve(data.size() * pressBit + 1);
	for (ValueType v : data) {
		for (int j = 0; j < pressBit; ++j) {
			buff.push_back(v % 10 + '0');
			v /= 10;
		}
	}
	while (buff.size() > 1 && buff.back() == '0')
		buff.pop_back();
	if (neg && !(buff.size() == 1 && buff[0] == '0')) buff.push_back('-');
	int n = buff.size();
	for (int i = 0; i < n - i - 1; ++i)
		std::swap(buff[i], buff[n - i - 1]);
	return buff;
}

std::ostream &operator<<(std::ostream &os, const Int &x) {
	std::vector<char> buff;
	buff.reserve(x.data.size() * pressBit);
	for (ValueType v : x.data) {
		for (int j = 0; j < pressBit; ++j) {
			buff.push_back(v % 10);
			v /= 10;
		}
	}
	while (buff.size() > 1 && buff.back() == 0)
		buff.pop_back();

	if (x.neg && !(buff.size() == 1 && buff[0] == 0)) os.put('-');
	for (auto cur = buff.rbegin(); cur != buff.rend(); ++cur)
		os.put(*cur + '0');
	return os;
}

bool cmpAbsLess(Int const &a, Int const &b) {
	if (a.len() < b.len())
		return true;
	else if (a.len() > b.len())
		return false;
	else {
		for (auto i = a.len() - 1; ~i; --i) {
			if (a[i] < b[i])
				return true;
			else if (a[i] > b[i])
				return false;
		}
		return false;
	}
}

bool cmpAbsLessEq(Int const &a, Int const &b) {
	return !cmpAbsLess(b, a);
}

bool cmpAbsEq(Int const &a, Int const &b) {
	if (a.len() != b.len())
		return false;
	for (auto i = a.len() - 1; ~i; --i)
		if (a[i] != b[i])
			return false;
	return true;
}

bool operator==(const Int &a, const Int &b) {
	if (a.neg == b.neg)
		return cmpAbsEq(a, b);
	else
		return false;
}

bool operator!=(const Int &a, const Int &b) {
	return !(a == b);
}

bool operator<(const Int &a, const Int &b) {
	if (a.neg != b.neg) return a.neg && !b.neg;
	bool positive = !a.neg;
	if (a.len() < b.len())
		return positive;
	else if (a.len() > b.len())
		return !positive;
	else {
		for (auto i = a.len() - 1; ~i; --i) {
			if (a[i] < b[i])
				return positive;
			else if (a[i] > b[i])
				return !positive;
		}
		return false;
	}
}

bool operator>(const Int &a, const Int &b) {
	return b < a;
}

bool operator<=(const Int &a, const Int &b) {
	return !(b < a);
}

bool operator>=(const Int &a, const Int &b) {
	return !(a < b);
}

__int128 Int::getFrontBits(int k) const {
	__int128 ret = 0;
	for (int i = 0; i < k && i < data.size(); ++i)
		ret = ret * P + data[data.size() - i - 1];
	return ret;
}

} // namespace kupi
