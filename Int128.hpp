#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>

class Int128 {
public:
    Int128();
    Int128(int64_t value);
    explicit Int128(std::string_view sv);

    explicit operator int64_t() const;
    explicit operator double() const;

    std::string str() const;

    Int128 operator-() const;

    Int128& operator+=(const Int128& rhs);
    Int128& operator-=(const Int128& rhs);
    Int128& operator*=(const Int128& rhs);
    Int128& operator/=(const Int128& rhs);

    bool operator==(const Int128& rhs) const;
    bool operator!=(const Int128& rhs) const;

private:
    uint64_t hi_;
    uint64_t lo_;
    bool negative_;

    Int128(uint64_t hi, uint64_t lo, bool neg);

    bool is_negative() const;
    bool is_zero() const;
    Int128 negate() const;
    int compare_abs(const Int128& other) const;
    int compare_signed(const Int128& other) const;
    Int128 add_abs(const Int128& other) const;
    Int128 sub_abs(const Int128& other) const;
    Int128 mul_abs(const Int128& other) const;
    std::pair<Int128, Int128> divmod_abs(const Int128& divisor) const;

    friend Int128 operator<<(const Int128& lhs, int bits);
    friend Int128 operator>>(const Int128& lhs, int bits);
    friend bool operator<(const Int128& lhs, const Int128& rhs);
    friend bool operator>(const Int128& lhs, const Int128& rhs);
    friend bool operator<=(const Int128& lhs, const Int128& rhs);
    friend bool operator>=(const Int128& lhs, const Int128& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Int128& val);
};

Int128 operator+(Int128 lhs, const Int128& rhs);
Int128 operator-(Int128 lhs, const Int128& rhs);
Int128 operator*(Int128 lhs, const Int128& rhs);
Int128 operator/(Int128 lhs, const Int128& rhs);

std::ostream& operator<<(std::ostream& os, const Int128& val);
