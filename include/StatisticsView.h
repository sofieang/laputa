#ifndef __STATISTICSVIEW_H__
#define __STATISTICSVIEW_H__

#include "Prefix.h"
#include <vector>
#include <string>

#include "Simulation.h"
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_PNG_Image.H>
#include <gsl/gsl_multifit_nlin.h>
#include "StatisticsBlock.h"

// curve types
#define N_CURVE_TYPES 7

#define MAX_CURVE_PARAMETERS 4

// curve class
class StatisticsView;
class Curve {
public:
	t_int type;
	t_float parameters[MAX_CURVE_PARAMETERS];
	gsl_multifit_function_fdf fdf;

	StatisticsView* view;

	t_int nParameters(void);
	t_float getParameterValue(t_int p);
	Fl_PNG_Image* getFormula(void);
	const char* getParameterName(t_int p);
};




// StatisticsView - class to display bar graph data & fit it to curves

class StatisticsView : public Fl_Widget {
public:
	// constructor & destructor
	StatisticsView(t_int x, t_int y, t_int w, t_int h, const char *label = 0);

	// draw method
	void draw(void);

	// get labels
	const char* xLabel(t_float x);
	const char* yLabel(t_float y);

	// set data to display
	void setData(const vector<t_int>& d , t_int m = 0);
	void setData(const vector<t_float>& d, t_float m = 0);
	void setData(const StatisticsBlock& b, t_float m = 0);
	void clear(void);
	void range(t_int l, t_int r);

	// curve fitting
	t_float curveError(Curve& c);
	void drawCurve(void);
	Curve findBestCurve(void);
	Curve fitToCurve(t_int type);

	// show as discrete or continuous?
	bool discrete;

	// curve to plot
	Curve curve;

	// interval of data to show
	t_int right, left;

	vector<t_float> data;
	t_float max, total;
};





// Functions for handling curve fitting
void FillOutCurveMenu(Fl_Choice* menu);
const char *GetCurveParameterName(t_int distr, t_int p);











#endif
