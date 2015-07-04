/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef number_DEFINED
#define number_DEFINED
class Number {
public:
    explicit Number(int v) : intValue(static_cast<int64_t>(v)), type(intType) {}
    explicit Number(double v) : doubleValue(v), type(doubleType) {}
    explicit Number(int64_t v) : intValue(v), type(intType) {}
    explicit Number(const std::string& s) {
        std::istringstream buffer(s);
        if (nullptr != strchr(s.c_str(), '.') ||
            nullptr != strchr(s.c_str(), 'E') ||
            nullptr != strchr(s.c_str(), 'e')) {
            buffer >> doubleValue;
            type = doubleType;
        } else {
            buffer >> intValue;
            type = intType;
        }
    }
    void serialize(std::ostream* o) const {
        switch (type) {
            case intType: *o << intValue; return;
            case doubleType: *o << doubleValue; return;
            default: assert(false);
        }
    }
    Number operator+(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return Number(intValue + rhs.intValue);
        }
        return Number(double(*this) + double(rhs));
    }
    Number operator-(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return Number(intValue - rhs.intValue);
        }
        return Number(double(*this) - double(rhs));
    }
    Number operator*(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return Number(intValue * rhs.intValue);
        }
        return Number(double(*this) * double(rhs));
    }
    Number operator/(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return Number(intValue / rhs.intValue);
        }
        return Number(double(*this) / double(rhs));
    }
    Number operator%(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return Number(intValue % rhs.intValue);
        }
        return Number(std::fmod(double(*this), double(rhs)));
    }
    bool operator==(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return intValue == rhs.intValue;
        }
        return double(*this) == double(rhs);
    }
    bool operator!=(Number rhs) const { return !(*this == rhs); }
    bool operator>(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return intValue > rhs.intValue;
        }
        return double(*this) > double(rhs);
    }
    bool operator<(Number rhs) const {
        if (BothInts(*this, rhs)) {
            return intValue < rhs.intValue;
        }
        return double(*this) < double(rhs);
    }
    bool operator>=(Number rhs) const { return !(*this < rhs); }
    bool operator<=(Number rhs) const { return !(*this > rhs); }
    Number& operator+=(Number rhs) { return *this = *this + rhs; }
    Number& operator-=(Number rhs) { return *this = *this - rhs; }
    Number& operator*=(Number rhs) { return *this = *this * rhs; }
    Number& operator/=(Number rhs) { return *this = *this / rhs; }
    Number& operator%=(Number rhs) { return *this = *this % rhs; }

private:
    union {
        int64_t intValue;
        double doubleValue;
    };
    enum Type {
        intType = 0,
        doubleType,
    } type;
    operator double() const {
        switch (type) {
            case intType: return static_cast<double>(intValue);
            case doubleType: return doubleValue;
            default: assert(false); return 0.0;
        }
    }
    static bool BothInts(Number u, Number v) {
        return u.type == intType && v.type == intType;
    }
};

#endif  // number_DEFINED
