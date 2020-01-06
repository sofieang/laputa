#include "Utility.h"
#include <stdlib.h>
#include <FL/Fl.H>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_math.h>
//-----------------------------------------------------------------------------------------------------------------------

const char* DoubleToString(double v, t_int digits) {
	static char str[256];
	if(gsl_isnan(v)) sprintf(str, " ");
	else if(gsl_isinf(v) == 1) sprintf(str, "+inf");
	else if(gsl_isinf(v) == -1) sprintf(str, "-inf");
	else {
		sprintf(str, "%.12f", v + .5 * pow((t_float)10, -digits));
		// find decimal point
		char* dec = strchr(str, '.');
		if(dec) {
			if(digits == 0) dec[digits] = 0;
			else dec[digits + 1] = 0;
		}
	}

	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

const char* IntToString(t_int v) {
	static char str[4096];
	sprintf(str, "%d", v);
	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

const char* WCharToString(wchar_t c) {
	static char str[4096];
	fl_utf8fromwc(str, 256, &c, 1);
	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

const char* StrConcat(const char* s1, const char* s2) {
    static char str[4096];
    strcpy(str, s1);
    strcat(str, s2);
    return str;
}

//-----------------------------------------------------------------------------------------------------------------------

void MoveAllWidgetsTo(Fl_Group *window, t_int x, t_int y) {
	// first get x0, y0
	t_int x0 = 0x7FFF, y0 = 0x7FFF;
	for(t_int i = 0; i < window->children(); ++i) {
		if(window->child(i)->x() < x0) x0 = window->child(i)->x();
		if(window->child(i)->y() < y0) y0 = window->child(i)->y();
	}
	t_int dx = x - x0, dy = y - y0;

	// move all children
	for(t_int i = 0; i < window->children(); ++i) window->child(i)->position(window->child(i)->x() + dx, window->child(i)->y() + dy);
}
