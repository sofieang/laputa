
#ifndef __UTILITY_H__
#define __UTILITY_H__
#include "Prefix.h"
#include "FL/Fl_Group.H"
#include <cmath>
#include <cstring>
#include <limits>

void MoveAllWidgetsTo(Fl_Group *window, t_int x, t_int y);
const char* DoubleToString(double v, t_int digits = 3);
const char* IntToString(t_int v);
const char* WCharToString(wchar_t c);
inline t_int Round(t_float v) {
	t_float i;
	if(modf(v, &i) >= 0.5) return (t_int)v + 1;
	else return (t_int)v;
}
const char* StrConcat(const char* s1, const char* s2);
void MakeDirectory(const char *dirName);

#define ISNAN(x) (gsl_isnan(x))
#define ISAN(x) (!gsl_isnan(x))
#define QNAN (numeric_limits<t_float>::quiet_NaN())
#define NEGINF (-numeric_limits<t_float>::infinity())
#define POSINF (numeric_limits<t_float>::infinity())


#endif
