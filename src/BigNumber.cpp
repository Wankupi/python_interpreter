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
			if (b & 1) r = static_cast<int>((long long)r * x % mod);
			x = static_cast<int>((long long)x * x % mod);
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

	void DFT(std::vector<int> &f, int op) const {
		std::vector<int> const &rev = revs[limit];
		for (int i = 0; i != limit; ++i)
			if (rev[i] < i) std::swap(f[i], f[rev[i]]);
		for (int len = 1; len < limit; len <<= 1) {
			int W1 = W(1, 2 * len, op);
			for (int l = 0; l < limit; l += len * 2) {
				int ome = 1;
				for (int i = 0; i < len; ++i) {
					int x = f[l + i], y = f[l + len + i];
					f[l + i] = pls(x, static_cast<int>((long long)ome * y % mod));
					f[l + len + i] = dec(x, static_cast<int>((long long)ome * y % mod));
					ome = static_cast<int>((long long)ome * W1 % mod);
				}
			}
		}
		if (op == -1) {
			long long inv = pow(limit, mod - 2);
			for (int i = 0; i < limit; ++i)
				f[i] = static_cast<int>(f[i] * inv % mod);
		}
	}

public:
	int limit;
	static std::map<int, std::vector<int>> revs;
};

std::map<int, std::vector<int>> FFT::revs;

void Int::remove_front_zero() {
	while (data.back() == 0 && data.size() > 1)
		data.pop_back();
	if (data.size() == 1 && data[0] == 0) neg = false;
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
	this->read(str);
}

Int Int::operator-() const { return {!neg, data}; }

Int Int::operator+() const { return *this; }

Int &Int::operator<<=(int x) {
	if (x <= 0) return *this;
	data.resize(data.size() + x);
	for (int i = data.size() - 1; i >= x; --i)
		data[i] = data[i - x];
	for (int i = 0; i < x; ++i)
		data[i] = 0;
	return *this;
}

Int &Int::operator>>=(int x) {
	if (x <= 0) return *this;
	if (x >= data.size()) {
		data = {0};
		return *this;
	}
	for (int i = 0; i + x < data.size(); ++i)
		data[i] = data[i + x];
	data.resize(data.size() - x);
	return *this;
}

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
		mxlen = static_cast<int>(data.size());

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
		fa[i] = static_cast<int>((long long)(fa[i]) * fb[i] % FFT::mod);
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

Int div(Int const &a, Int const &b) {
	if (b == 1) return a;
	std::vector<Int> sta;
	std::vector<Int> s;
	Int ret = b, tem = 1;
	while (ret <= a) {
		sta.push_back(ret);
		s.push_back(tem);
		ret += ret;
		tem += tem;
	}
	ret = a;
	tem = 0;
	int len = sta.size() - 1;
	while (len >= 0) {
		Int tmp = sta[len];
		if (tmp <= ret) ret -= tmp, tem += s[len];
		len--;
	}
	return tem;
}

Int getInverse(Int const &a) {
	int m = a.len();
	if (m <= 9)
		return div(Int(1) <<= (m << 1), a);
	int k = (m + 5) >> 1;
	Int b = a;
	b >>= (m - k);
	Int c = getInverse(b);
	Int c1 = c + c;
	c1 <<= (m - k);
	b = a * c * c;
	b >>= (k << 1);
	Int d = c1 - b;
	d -= 1;
	c = (Int(1) <<= (m << 1)) - d * a;
	if (c >= a) d += 1;
	return d;
}

Int operator/(const Int &a, const Int &b) {
	int l1 = a.data.size(), l2 = b.data.size();
	if (l1 < l2)
		return -(a.neg != b.neg);
	if (a.len() <= 17 && b.len() <= 17) {
		__int128 v1 = a.getFrontBits(17), v2 = b.getFrontBits(17);
		if (a.neg == b.neg) return v1 / v2;
		return -(v1 + v2 - 1) / v2;
	}
	bool new_neg = a.neg != b.neg;
	Int x = a, y = b;
	x.neg = y.neg = false;
	if (new_neg) x += y - 1;
	if (l1 > (l2 << 1)) {
		int tmp = l1 - (l2 << 1);
		x <<= tmp;
		y <<= tmp;
		l2 = l1 - l2;
		l1 = l2 << 1;
	}
	Int z = getInverse(y);
	y = x * z;
	y >>= (l2 << 1);
	x = b * y;
	x.neg = a.neg;
	Int ret = a - x;
	if (cmpAbsLessEq(b, ret)) y += Int(1);
	y.neg = new_neg;
	return y;
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
			buff.push_back(static_cast<char>(v % 10 + '0'));
			v /= 10;
		}
	}
	while (buff.size() > 1 && buff.back() == '0')
		buff.pop_back();
	if (neg && !(buff.size() == 1 && buff[0] == '0')) buff.push_back('-');
	int n = static_cast<int>(buff.size());
	for (int i = 0; i < n - i - 1; ++i)
		std::swap(buff[i], buff[n - i - 1]);
	return buff;
}

void Int::read(std::string const &str) {
	data.clear();
	neg = false;
	int ed = str.find_first_of('.');
	if (ed == std::string::npos) ed = str.length();
	--ed;
	int bg = str.find_first_of('-');
	if (bg == std::string::npos || bg > ed) bg = 0;
	data.reserve((ed - bg + 1 + pressBit - 1) / pressBit);
	ValueType val = 0;
	int haveBits = 0;
	for (auto cur = ed; cur >= bg; --cur) {
		char c = str[cur];
		if (!isdigit(c)) {
			if (c == '-') neg = true;
			break;
		}
		val = val + pow_10(haveBits) * (c - '0');
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

void Int::print() const {
	std::cout << *this;
}

std::istream &operator>>(std::istream &is, Int &x) {
	std::string tmp;
	is >> tmp;
	x.read(tmp);
	return is;
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
