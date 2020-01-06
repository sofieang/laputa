#ifndef __AMOUNT_H__
#define __AMOUNT_H__

#include "Prefix.h"
#include <assert.h>

#define amt_type double


// special class to avoid problems with beliefs etc. very close to 0 or 1
// values in the range [0, 0.5] are stored as is
// values in the range (0.5, 1.0] are stored as value - 1.0
// since 0 is both 0 and 1, the sign bit is used to tell the difference. +0 is zero, -0 is one.

class Amount {
public:
	Amount(void) : upper(.5), lower(.5) {}
	Amount(amt_type val) {
		assert(val >= 0 && val <= 1.0);
		upper = 1.0 - val;
		lower = val;
	}
	Amount(const Amount& val) {
		upper = val.upper;
		lower = val.lower;
	}

	// assignment
	inline Amount operator=(const Amount val) {
		upper = val.upper;
		lower = val.lower;
		return *this;
	}
	inline Amount operator=(t_float val) {
		set(val);
		return *this;
	}

	// access the value as t_float
	inline amt_type v(void) const {
		return 0.5 + 0.5 * (lower - upper);
	}

	inline void set(amt_type val) {
		assert(val >= 0 && val <= 1.0);
		upper = 1.0 - val;
		lower = val;
	}

	// do 1 - value
	inline Amount inverted(void) const {
		Amount v;
		v.upper = lower;
		v.lower = upper;
		return v;
	}

	// operators
	inline Amount operator+=(const Amount val) {
		lower += val.lower;
		upper += val.upper - 1.0;
		assert(lower >= 0 && lower <= 1.0 && upper >= 0 && upper <= 1.0);
		return *this;
	}

	inline Amount operator-=(const Amount val) {
		lower -= val.lower;
		upper -= val.upper - 1.0;
		assert(lower >= 0 && lower <= 1.0 && upper >= 0 && upper <= 1.0);
		return *this;
	}

	inline Amount operator*=(const Amount val) {
		lower *= val.lower;
		upper = upper + val.upper - upper * val.upper;
		assert(lower >= 0 && lower <= 1.0 && upper >= 0 && upper <= 1.0);
		return *this;
	}

	inline Amount operator/=(const Amount val) {
		lower /= val.lower;
		upper = (upper - val.upper) / (1.0 - val.upper);
		assert(lower >= 0 && lower <= 1.0 && upper >= 0 && upper <= 1.0);
		return *this;
	}

	// function to compute this / (this + val)
	inline Amount dividedByAdded(Amount val) const {
		Amount amt;
		amt.lower = lower / (lower + val.lower);
		amt.upper = (1.0 - val.upper) / (2.0 - upper - val.upper);
		return amt;
	}

	// friends
	friend bool operator==(const Amount lhs, const Amount rhs);
	friend bool operator!=(const Amount lhs, const Amount rhs);
	friend bool operator<(const Amount lhs, const Amount rhs);
	friend bool operator<=(const Amount lhs, const Amount rhs);
	friend bool operator>(const Amount lhs, const Amount rhs);
	friend bool operator>=(const Amount lhs, const Amount rhs);


private:
	amt_type upper;			// 1 - value
	amt_type lower;			// value
};

// binary operators
inline Amount operator+(const Amount amt1, const Amount amt2) {
	Amount amt(amt1);
	amt += amt2;
	return amt;
}
inline Amount operator-(const Amount amt1, const Amount amt2) {
	Amount amt(amt1);
	amt -= amt2;
	return amt;
}
inline Amount operator*(const Amount amt1, const Amount amt2) {
	Amount amt(amt1);
	amt *= amt2;
	return amt;
}
inline Amount operator/(const Amount amt1, const Amount amt2) {
	Amount amt(amt1);
	amt /= amt2;
	return amt;
}

// comparison
inline bool operator==(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper == rhs.lower - rhs.upper;
}
inline bool operator!=(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper != rhs.lower - rhs.upper;
}
inline bool operator<(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper < rhs.lower - rhs.upper;
}

inline bool operator>(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper > rhs.lower - rhs.upper;
}

inline bool operator<=(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper <= rhs.lower - rhs.upper;
}

inline bool operator>=(const Amount lhs, const Amount rhs) {
	return lhs.lower - lhs.upper >= rhs.lower - rhs.upper;
}


#endif
