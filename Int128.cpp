#include "Int128.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cmath>

namespace {

uint64_t adc(uint64_t a, uint64_t b, uint64_t& carry) {
    uint64_t sum = a + b;
    carry = (sum < a) ? 1 : 0;
    return sum;
}

uint64_t sbb(uint64_t a, uint64_t b, uint64_t& borrow) {
    uint64_t diff = a - b;
    borrow = (a < b) ? 1 : 0;
    return diff;
}

std::pair<uint64_t, uint64_t> umul64(uint64_t a, uint64_t b) {
    uint64_t a0 = a & 0xFFFFFFFFULL;
    uint64_t a1 = a >> 32;
    uint64_t b0 = b & 0xFFFFFFFFULL;
    uint64_t b1 = b >> 32;

    uint64_t p00 = a0 * b0;
    uint64_t p01 = a0 * b1;
    uint64_t p10 = a1 * b0;
    uint64_t p11 = a1 * b1;

    uint64_t mid = p01 + (p00 >> 32);
    uint64_t carry = (mid < p01) ? 1 : 0;
    mid += p10;
    carry += (mid < p10) ? 1 : 0;

    uint64_t lo = (mid << 32) | (p00 & 0xFFFFFFFFULL);
    uint64_t hi = p11 + (mid >> 32) + carry;

    return {hi, lo};
}

void shl128(uint64_t& hi, uint64_t& lo, int n) {
    if (n <= 0) return;
    if (n >= 128) { hi = 0; lo = 0; return; }
    if (n >= 64) {
        hi = lo << (n - 64);
        lo = 0;
    } else {
        uint64_t carry = lo >> (64 - n);
        lo <<= n;
        hi = (hi << n) | carry;
    }
}

void shr128(uint64_t& hi, uint64_t& lo, int n) {
    if (n <= 0) return;
    if (n >= 128) { hi = 0; lo = 0; return; }
    if (n >= 64) {
        lo = hi >> (n - 64);
        hi = 0;
    } else {
        uint64_t carry = hi << (64 - n);
        hi >>= n;
        lo = (lo >> n) | carry;
    }
}

int cmp_abs(uint64_t a_hi, uint64_t a_lo, uint64_t b_hi, uint64_t b_lo) {
    if (a_hi != b_hi) return (a_hi < b_hi) ? -1 : 1;
    if (a_lo != b_lo) return (a_lo < b_lo) ? -1 : 1;
    return 0;
}

}

Int128::Int128() : hi_(0), lo_(0), negative_(false) {}

Int128::Int128(int64_t value) : negative_(value < 0) {
    uint64_t mag = static_cast<uint64_t>(value < 0 ? -value : value);
    hi_ = 0;
    lo_ = mag;
}

Int128::Int128(uint64_t hi, uint64_t lo, bool neg)
    : hi_(hi), lo_(lo), negative_(neg) {}

Int128::Int128(std::string_view text) : Int128() {
    if (text.empty()) return;
    size_t pos = 0;
    if (text[0] == '-') {
        negative_ = true;
        pos = 1;
    } else if (text[0] == '+') {
        pos = 1;
    }
    for (; pos < text.size(); ++pos) {
        unsigned char ch = static_cast<unsigned char>(text[pos]);
        if (!std::isdigit(ch)) break;
        *this = *this * Int128(10) + Int128(ch - '0');
    }
}

Int128::operator int64_t() const {
    if (hi_ != 0) {
        throw std::out_of_range("Int128 out of int64_t range");
    }
    uint64_t mag = lo_;
    if (negative_ && mag > 9223372036854775808ULL) {
        throw std::out_of_range("Int128 out of int64_t range");
    }
    if (!negative_ && mag > 9223372036854775807ULL) {
        throw std::out_of_range("Int128 out of int64_t range");
    }
    return negative_ ? -static_cast<int64_t>(mag) : static_cast<int64_t>(mag);
}

Int128::operator double() const {
    if (is_zero()) return 0.0;
    double magnitude = std::ldexp(static_cast<double>(hi_), 64) + static_cast<double>(lo_);
    return negative_ ? -magnitude : magnitude;
}

std::string Int128::str() const {
    if (is_zero()) return "0";
    std::string digits;
    Int128 tmp = *this;
    bool neg = tmp.negative_;
    if (neg) tmp = tmp.negate();
    while (!tmp.is_zero()) {
        auto [q, r] = tmp.divmod_abs(Int128(10));
        digits.push_back('0' + static_cast<char>(r.lo_));
        tmp = q;
    }
    if (neg) digits.push_back('-');
    std::reverse(digits.begin(), digits.end());
    return digits;
}

bool Int128::is_negative() const {
    return negative_ && !is_zero();
}

bool Int128::is_zero() const {
    return hi_ == 0 && lo_ == 0;
}

Int128 Int128::negate() const {
    if (is_zero()) return *this;
    return Int128(hi_, lo_, !negative_);
}

Int128 Int128::operator-() const {
    return negate();
}

int Int128::compare_abs(const Int128& other) const {
    return cmp_abs(hi_, lo_, other.hi_, other.lo_);
}

int Int128::compare_signed(const Int128& other) const {
    bool a_neg = is_negative();
    bool b_neg = other.is_negative();
    if (a_neg != b_neg) return a_neg ? -1 : 1;
    int c = compare_abs(other);
    return a_neg ? -c : c;
}

Int128 Int128::add_abs(const Int128& other) const {
    uint64_t carry = 0;
    uint64_t new_lo = adc(lo_, other.lo_, carry);
    uint64_t new_hi = hi_ + other.hi_ + carry;
    return Int128(new_hi, new_lo, negative_);
}

Int128& Int128::operator+=(const Int128& rhs) {
    bool a_neg = is_negative();
    bool b_neg = rhs.is_negative();
    if (a_neg == b_neg) {
        uint64_t carry = 0;
        uint64_t nlo = adc(lo_, rhs.lo_, carry);
        uint64_t nhi = hi_ + rhs.hi_ + carry;
        hi_ = nhi;
        lo_ = nlo;
    } else {
        int c = compare_abs(rhs);
        if (c == 0) {
            hi_ = 0; lo_ = 0; negative_ = false;
            return *this;
        }
        if (c > 0) {
            uint64_t borrow = 0;
            lo_ = sbb(lo_, rhs.lo_, borrow);
            hi_ -= rhs.hi_ + borrow;
        } else {
            uint64_t borrow = 0;
            lo_ = sbb(rhs.lo_, lo_, borrow);
            hi_ = rhs.hi_ - hi_ - borrow;
            negative_ = b_neg;
        }
    }
    return *this;
}

Int128 operator+(Int128 lhs, const Int128& rhs) {
    return lhs += rhs;
}

Int128& Int128::operator-=(const Int128& rhs) {
    *this += rhs.negate();
    return *this;
}

Int128 operator-(Int128 lhs, const Int128& rhs) {
    return lhs -= rhs;
}

Int128 Int128::mul_abs(const Int128& other) const {
    auto [h1, l1] = umul64(lo_, other.lo_);
    auto [h2, l2] = umul64(lo_, other.hi_);
    auto [h3, l3] = umul64(hi_, other.lo_);

    uint64_t carry = 0;
    uint64_t res_lo = adc(l1, l2, carry);
    uint64_t res_hi = h1 + h2 + carry;
    carry = 0;
    res_lo = adc(res_lo, l3, carry);
    res_hi += h3 + carry;

    return Int128(res_hi, res_lo, false);
}

Int128& Int128::operator*=(const Int128& rhs) {
    bool res_neg = (is_negative() != rhs.is_negative()) && !rhs.is_zero() && !is_zero();
    Int128 product = mul_abs(rhs);
    hi_ = product.hi_;
    lo_ = product.lo_;
    negative_ = res_neg;
    return *this;
}

Int128 operator*(Int128 lhs, const Int128& rhs) {
    return lhs *= rhs;
}

std::pair<Int128, Int128> Int128::divmod_abs(const Int128& divisor) const {
    if (divisor.is_zero()) return {Int128(0), *this};

    int cmp = compare_abs(divisor);
    if (cmp < 0) return {Int128(0), *this};
    if (cmp == 0) return {Int128(1), Int128(0)};

    uint64_t q_hi = 0, q_lo = 0;
    uint64_t r_hi = 0, r_lo = 0;

    for (int i = 127; i >= 0; --i) {
        shl128(r_hi, r_lo, 1);
        uint64_t bit = (i >= 64) ? (hi_ >> (i - 64)) & 1ULL : (lo_ >> i) & 1ULL;
        if (bit) r_lo |= 1;
        if (cmp_abs(r_hi, r_lo, divisor.hi_, divisor.lo_) >= 0) {
            uint64_t borrow = 0;
            r_lo = sbb(r_lo, divisor.lo_, borrow);
            r_hi -= divisor.hi_ + borrow;
            if (i >= 64) {
                q_hi |= (1ULL << (i - 64));
            } else {
                q_lo |= (1ULL << i);
            }
        }
    }
    return {Int128(q_hi, q_lo, false), Int128(r_hi, r_lo, false)};
}

Int128& Int128::operator/=(const Int128& rhs) {
    bool res_neg = (is_negative() != rhs.is_negative()) && !is_zero();
    Int128 quot = divmod_abs(rhs).first;
    hi_ = quot.hi_;
    lo_ = quot.lo_;
    negative_ = res_neg;
    return *this;
}

Int128 operator/(Int128 lhs, const Int128& rhs) {
    return lhs /= rhs;
}

Int128 operator<<(const Int128& lhs, int bits) {
    uint64_t h = lhs.hi_, l = lhs.lo_;
    shl128(h, l, bits);
    return Int128(h, l, lhs.negative_);
}

Int128 operator>>(const Int128& lhs, int bits) {
    uint64_t h = lhs.hi_, l = lhs.lo_;
    shr128(h, l, bits);
    return Int128(h, l, lhs.negative_);
}

bool Int128::operator==(const Int128& rhs) const {
    return hi_ == rhs.hi_ && lo_ == rhs.lo_ && negative_ == rhs.negative_;
}

bool Int128::operator!=(const Int128& rhs) const {
    return !(*this == rhs);
}

bool operator<(const Int128& a, const Int128& b) {
    return a.compare_signed(b) < 0;
}

bool operator>(const Int128& a, const Int128& b) {
    return a.compare_signed(b) > 0;
}

bool operator<=(const Int128& a, const Int128& b) {
    return a.compare_signed(b) <= 0;
}

bool operator>=(const Int128& a, const Int128& b) {
    return a.compare_signed(b) >= 0;
}

std::ostream& operator<<(std::ostream& os, const Int128& val) {
    return os << val.str();
}
