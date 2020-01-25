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
	Amount(void) : inverse(.5), value(.5) {}
	Amount(amt_type val) {
		assert(val >= 0 && val <= 1.0);
		inverse = 1.0 - val;
		value = val;
	}
	Amount(const Amount& val) {
		inverse = val.inverse;
		value = val.value;
	}

	// assignment
	inline Amount operator=(const Amount val) {
		inverse = val.inverse;
		value = val.value;
		return *this;
	}
	inline Amount operator=(t_float val) {
		set(val);
		return *this;
	}

	// access the value as t_float
	inline amt_type v(void) const {
		return 0.5 + 0.5 * (value - inverse);
	}

	inline void set(amt_type val) {
		assert(val >= 0 && val <= 1.0);
		inverse = 1.0 - val;
		value = val;
	}

	// do 1 - value
	inline Amount inverted(void) const {
		Amount v;
		v.inverse = value;
		v.value = inverse;
		return v;
	}

	// operators
	inline Amount operator+=(const Amount val) {
		value += val.value;
		inverse += val.inverse - 1.0;
		assert(value >= 0 && value <= 1.0 && inverse >= 0 && inverse <= 1.0);
		return *this;
	}

	inline Amount operator-=(const Amount val) {
		value -= val.value;
		inverse -= val.inverse - 1.0;
		assert(value >= 0 && value <= 1.0 && inverse >= 0 && inverse <= 1.0);
		return *this;
	}

	inline Amount operator*=(const Amount val) {
		value *= val.value;
		inverse = inverse + val.inverse - inverse * val.inverse;
		assert(value >= 0 && value <= 1.0 && inverse >= 0 && inverse <= 1.0);
		return *this;
	}

	inline Amount operator/=(const Amount val) {
		if (val.value > 0.5) {
			value /= val.value;
			inverse = (inverse - val.inverse) / (1.0 - val.inverse);
		}
		else {
			inverse = (inverse - val.inverse) / (1.0 - val.inverse);
			value = 1.0 - inverse;
		}
		assert(value >= 0 && value <= 1.0 && inverse >= 0 && inverse <= 1.0);
		return *this;
	}

	// function to compute this / (this + val)
	inline Amount dividedByAdded(Amount val) const {
		Amount amt;
		if (inverse + val.inverse < 1.0) {
			amt.inverse = (1.0 - val.inverse) / (2.0 - inverse - val.inverse);
			amt.value = 1.0 - amt.inverse;
		}
		else {
			amt.value = value / (value + val.value);
			amt.inverse = 1.0 - amt.value;
		}
		assert(amt.value >= 0 && amt.value <= 1.0 && amt.inverse >= 0 && amt.inverse <= 1.0);
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
	amt_type inverse;		// 1 - value
	amt_type value;			// value
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
	return lhs.value - lhs.inverse == rhs.value - rhs.inverse;
}
inline bool operator!=(const Amount lhs, const Amount rhs) {
	return lhs.value - lhs.inverse != rhs.value - rhs.inverse;
}
inline bool operator<(const Amount lhs, const Amount rhs) {
	return lhs.value - lhs.inverse < rhs.value - rhs.inverse;
}

inline bool operator>(const Amount lhs, const Amount rhs) {
	return lhs.value - lhs.inverse > rhs.value - rhs.inverse;
}

inline bool operator<=(const Amount lhs, const Amount rhs) {
	return lhs.value - lhs.inverse <= rhs.value - rhs.inverse;
}

inline bool operator>=(const Amount lhs, const Amount rhs) {
	return lhs.value - lhs.inverse >= rhs.value - rhs.inverse;
}


#endif
