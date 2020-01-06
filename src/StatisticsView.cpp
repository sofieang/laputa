
#include "App.h"
#include "StatisticsView.h"
#include <FL/fl_draw.H>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>
#include "Utility.h"

#define MAX_PARAMETERS 4

// parameter types
#define P_LIN 0
#define P_EXP 1
#define P_SQR 2

struct Parameter {
	wchar_t symbol;
	t_float guess;
	int scale;
};

struct CurveType {
	char name[64];
	char picture[64];
	int minCutoff;
	int nParameters;
	Parameter param[MAX_PARAMETERS];
	int (*gsl_value)(const gsl_vector *x, void *params, gsl_vector *f);
	Fl_PNG_Image *img;
};


int gsl_value_normal(const gsl_vector *x, void *params, gsl_vector *f);
int gsl_value_poisson(const gsl_vector *x, void *params, gsl_vector *f);
int gsl_value_zipf_mandelbrot(const gsl_vector *x, void *params, gsl_vector *f);
int gsl_value_yule_simon(const gsl_vector *x, void *params, gsl_vector *f);
int gsl_value_beta_prime(const gsl_vector *x, void *params, gsl_vector *f);
int gsl_value_extreme_value(const gsl_vector *x, void *params, gsl_vector *f);

struct CurveType curveTypes[N_CURVE_TYPES] = {
	{{"None"}, {"nodistribution.png"}, 0, 0, {{0, 0, P_LIN}, {0, 0, P_LIN}, {0, 0, P_LIN}, {0, 0, P_LIN}}, 0, 0},
	{{"Normal"}, {"normaldistribution.png"}, 0, 2, {{0x03BC, 5, P_LIN}, {0x03C3, 5, P_EXP}, {0, 0, P_LIN}, {0, 0, P_LIN}}, gsl_value_normal, 0},
	{{"Poisson"}, {"poissondistribution.png"}, 0, 1, {{0x03BB, 1, P_EXP}, {0, 0, P_LIN}, {0, 0, P_LIN}, {0, 0, P_LIN}}, gsl_value_poisson, 0},
	{{"Yule-Simon"}, {"yulesimondistribution.png"}, 1, 1, {{0x03C1, 1, P_EXP}, {0, 0, P_LIN}, {0, 0, P_LIN}, {0, 0, P_LIN}}, gsl_value_yule_simon, 0},
	{{"Zipf-Mandelbrot"}, {"zipfmandelbrotdistribution.png"}, 0, 3, {{0x03BA, .5, P_EXP}, {'q', 0.5, P_EXP}, {'s', 2, P_EXP}, {0, 0, P_LIN}}, gsl_value_zipf_mandelbrot, 0},
	{{"Generalised Extreme Value"}, {"generalisedextremevaluedistribution.png"}, 0, 3, {{0x03BE, 0, P_LIN}, {'m', 1, P_LIN}, {'s', 1, P_EXP}, {0, 0, P_LIN}}, gsl_value_extreme_value, 0},
	{{"Generalised Beta Prime"}, {"generalisedbetaprimedistribution.png"}, 1, 4, {{0x03B1, 1, P_EXP}, {0x03B2, 1, P_EXP}, {'p', 1, P_EXP}, {'q', 1, P_EXP}}, gsl_value_beta_prime, 0}
};


struct fDfData {
	int (*gsl_value)(const gsl_vector *x, void *params, gsl_vector *f);
	void *params;
};

//-----------------------------------------------------------------------------------------------------------------------

inline t_float linexp(t_float x) {
	if(x < 0) return exp(x);
	else return x + 1.0;
}

//-----------------------------------------------------------------------------------------------------------------------

inline t_float linexpinv(t_float y) {
	assert(y > 0);
	if(y < 1) return log(y);
	else return y - 1.0;
}

//-----------------------------------------------------------------------------------------------------------------------

inline t_float linsqr(t_float x) {
	if(x < -0.5) return -x - 0.25;
	else if(x < 0.5) return x * x;
	else return x - 0.25;
}

//-----------------------------------------------------------------------------------------------------------------------

inline t_float linsqrinv(t_float y) {
	assert(y >= 0);
	if(y < 0.25) return sqrt(y);
	else return y + 0.25;
}

//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_normal(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float mean = gsl_vector_get(x, 0);
	t_float dev = linexp(gsl_vector_get(x, 1));

	for(t_int k = 0; k < view->right - view->left; ++k) gsl_vector_set(f, k, gsl_ran_gaussian_pdf(k - mean, dev) - view->data[k + view->left] / view->total);
	return GSL_SUCCESS;
}


//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_poisson(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float lambda = linexp(gsl_vector_get(x, 0));

	for(t_int k = 0; k < view->right - view->left; ++k) gsl_vector_set(f, k, gsl_ran_poisson_pdf(k, lambda) - view->data[k + view->left] / view->total);
	return GSL_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_zipf_mandelbrot(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float kappa = linexp(gsl_vector_get(x, 0));
	t_float q = linexp(gsl_vector_get(x, 1));
	t_float s = linexp(gsl_vector_get(x, 2));


	// fill out values
	for(t_int k = 0; k < view->right - view->left; ++k) gsl_vector_set(f, k, kappa / pow(k + q, s) - view->data[k + view->left] / view->total);
	return GSL_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_yule_simon(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float rho = linexp(gsl_vector_get(x, 0));

	for(t_int k = 0; k < view->right - view->left; ++k) gsl_vector_set(f, k, rho * gsl_sf_beta(k + 1, rho + 1) - view->data[k + view->left] / view->total);
	return GSL_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_extreme_value(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float xi = gsl_vector_get(x, 0);
	t_float m = gsl_vector_get(x, 1);
	t_float s = linexp(gsl_vector_get(x, 2));

	for(t_int k = 0; k < view->right - view->left; ++k) {
		t_float kms = (k - m) / s, t;
		if(xi == 0) t = exp(-kms);
		else t = pow(1 + xi * kms, -1 / xi);
		gsl_vector_set(f, k, pow(t, xi + 1) * exp(-t) / s - view->data[k + view->left] / view->total);
	}
	return GSL_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------

int gsl_value_beta_prime(const gsl_vector *x, void *params, gsl_vector *f) {
	StatisticsView *view = ((Curve*)params)->view;
	t_float alpha = linexp(gsl_vector_get(x, 0));
	t_float beta = linexp(gsl_vector_get(x, 1));
	t_float p = linexp(gsl_vector_get(x, 2));
	t_float q = linexp(gsl_vector_get(x, 3));

	for(t_int k = 0; k < view->right - view->left; ++k) gsl_vector_set(f, k, p * pow(k / q, alpha * p - 1) * pow(1 + pow(k / q, p), -(alpha + beta)) / (q * gsl_sf_beta(alpha, beta)) - view->data[k + view->left] / view->total);
	return GSL_SUCCESS;
}


//-----------------------------------------------------------------------------------------------------------------------

void FillOutCurveMenu(Fl_Choice* menu) {
	menu->clear();
	for(t_int i = 0; i < N_CURVE_TYPES; ++i) menu->add(curveTypes[i].name, 0, 0);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Curve::getParameterValue(t_int p) {
	if(curveTypes[type].param[p].scale == P_EXP) return linexp(parameters[p]);
	else if(curveTypes[type].param[p].scale == P_SQR) return linsqr(parameters[p]);
	else return parameters[p];
}

//-----------------------------------------------------------------------------------------------------------------------

t_int Curve::nParameters(void) {
	return curveTypes[type].nParameters;
}

//-----------------------------------------------------------------------------------------------------------------------

const char *Curve::getParameterName(t_int p) {
	return WCharToString(curveTypes[type].param[p].symbol);
}

//-----------------------------------------------------------------------------------------------------------------------

Fl_PNG_Image* Curve::getFormula(void) {
	if(curveTypes[type].img) return curveTypes[type].img;
	else {
		char file[FL_PATH_MAX];
        strcpy(file, app->dataFile("formulae/").c_str());
		strcat(file, curveTypes[type].picture);
		curveTypes[type].img = new Fl_PNG_Image(file);
		assert(curveTypes[type].img);
		return curveTypes[type].img;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsView::StatisticsView(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Widget(x, y, w, h, label) {
	curve.type = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::draw(void) {
	// clear area
	fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);

	// draw data
	if(data.size() > 1) {
		fl_push_clip(x() + 1, y() + 1, w() - 1, h() - 1);
		for(t_int i = 0; i < data.size() - 1; ++i) {
			t_int x0 = x() + (w() - 1) * i / (data.size() - 1);
			t_int x1 = x() + (w() - 1)  * (i + 1) / (data.size() - 1) + 1;
			t_int y0 = y() + h() * (1.0 - data[i]), y1 = y() + h() * (1.0 - data[i + 1]);

			if(i >= left && i < right) fl_color(255, 0, 0);
			else fl_color(255, 128, 128);
			fl_polygon(x0, y() + h() - 1, x1, y() + h() - 1, x1, y1, x0, y0);

		}
		if(curve.type) drawCurve();
		fl_pop_clip();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::drawCurve(void) {
	if(curveTypes[curve.type].minCutoff > left) return;
	if(curveTypes[curve.type].nParameters >= right - left) return;

	curve.view = this;
	gsl_vector *v = gsl_vector_alloc(right - left);
	gsl_vector *p = gsl_vector_alloc(curveTypes[curve.type].nParameters);
	for(t_int i = 0; i < curveTypes[curve.type].nParameters; ++i) gsl_vector_set(p, i, curve.parameters[i]);
	(*curveTypes[curve.type].gsl_value) (p, &curve, v);
	t_float y0 = gsl_vector_get(v, 0) * total + data[left];

	fl_color(0, 0, 0);
	fl_line_style(FL_DASH, 2, 0);

	for(t_int i = 1; i < right - left; ++i) {
		t_float y1 = gsl_vector_get(v, i) * total + data[i + left];
		fl_line(x() + (i + left - 1) * w() / (data.size() - 1), y() + h() * (1 - y0), x() + (i + left) * w() / (data.size() - 1), y() + h() * (1 - y1));
		y0 = y1;
	}
	fl_line_style(FL_SOLID, 0, 0);

	gsl_vector_free(v);
	gsl_vector_free(p);
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::range(t_int l, t_int r) {
	left = l;
	right = r;
	total = 0;
	for(t_int i = left; i < right; ++i) total += data[i];
	if(total <= 0.00001) total = 0.00001;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::setData(const StatisticsBlock& b, t_float m) {
	// convert to vector representation
	assert(b.dim == 1);
	setData(b.toVector(), m);
}


//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::setData(const vector<t_float>& d, t_float m) {
	// find maximum
	max = NEGINF;
	total = 0;
	if(m != 0) max = m;
	else for(t_int i = 0; i < d.size(); ++i) if(d[i] > max) max = d[i];
	for(t_int i = 0; i < d.size(); ++i) total += d[i];

	// convert data
	data.resize(d.size());
	for(t_int i = 0; i < d.size(); ++i) data[i] = d[i] / max;
	total /= max;

	left = 0;
	right = data.size();
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::setData(const vector<t_int>& d, t_int m) {
	// find maximum
	max = NEGINF;
	total = 0;
	if(m != 0) max = m;
	else for(t_int i = 0; i < d.size(); ++i) if(d[i] > max) max = d[i];
	for(t_int i = 0; i < d.size(); ++i) total += d[i];

	// convert data
	data.resize(d.size());
	for(t_int i = 0; i < d.size(); ++i) data[i] = (t_float)d[i] / max;
	total /= max;

	left = 0;
	right = data.size();
}

//-----------------------------------------------------------------------------------------------------------------------

const char* StatisticsView::xLabel(t_float x) {
	return DoubleToString(x * (data.size() - 1), 1);
}

//-----------------------------------------------------------------------------------------------------------------------

const char* StatisticsView::yLabel(t_float y) {
	return DoubleToString(y * max, 1);

}

//-----------------------------------------------------------------------------------------------------------------------

t_float StatisticsView::curveError(Curve& c) {
	if(c.type) {
		c.view = this;
		if(left < curveTypes[c.type].minCutoff) return (t_float)GSL_POSINF;
		if(curveTypes[c.type].nParameters > right - left) return (t_float)GSL_POSINF;
		gsl_vector *v = gsl_vector_alloc(right - left);
		gsl_vector *p = gsl_vector_alloc(curveTypes[c.type].nParameters);
		for(t_int i = 0; i < curveTypes[c.type].nParameters; ++i) gsl_vector_set(p, i, c.parameters[i]);
		(*curveTypes[c.type].gsl_value) (p, &c, v);

		t_float err = 0;
		for(t_int i = 0; i < right - left; ++i) err += gsl_vector_get(v, i) * gsl_vector_get(v, i);

		gsl_vector_free(v);
		gsl_vector_free(p);
		return sqrt(err);
	}
	else return GSL_POSINF;
}


//-----------------------------------------------------------------------------------------------------------------------

Curve StatisticsView::findBestCurve(void) {
	Curve c;
	c.type = 0;
	t_float err = GSL_POSINF;
	for(t_int i = 1; i < N_CURVE_TYPES; ++i) {
		Curve d = fitToCurve(i);
		if(d.type) {
			if(curveError(d) < err) {
				c = d;
				err = curveError(d);
			}
		}
	}

	return c;
}

//-----------------------------------------------------------------------------------------------------------------------

Curve StatisticsView::fitToCurve(t_int type) {
	Curve c;
	c.type = type;
	c.view = this;
	c.fdf.f = curveTypes[type].gsl_value;
	c.fdf.df = 0;
	c.fdf.fdf = 0;
	c.fdf.n = right - left;
	c.fdf.p = curveTypes[type].nParameters;
	c.fdf.params = &c;
	if(curveTypes[type].nParameters > right - left) c.type = 0;
	if(left < curveTypes[type].minCutoff) c.type = 0;
	if(c.type == 0) return c;

	// set up solver
	gsl_multifit_fdfsolver *solver = gsl_multifit_fdfsolver_alloc(gsl_multifit_fdfsolver_lmder, right - left, curveTypes[type].nParameters);
	gsl_vector *x = gsl_vector_alloc(curveTypes[type].nParameters);
	for (t_int i = 0; i < curveTypes[type].nParameters; ++i) {
		if(curveTypes[type].param[i].scale == P_EXP) gsl_vector_set(x, i, linexpinv(curveTypes[type].param[i].guess));
		else if(curveTypes[type].param[i].scale == P_SQR) gsl_vector_set(x, i, linsqrinv(curveTypes[type].param[i].guess));
		else gsl_vector_set(x, i, curveTypes[type].param[i].guess);
	}
	gsl_multifit_fdfsolver_set(solver, &c.fdf, x);

	// solve for parameter values
	t_int result = GSL_CONTINUE;
	for(int iter = 0; iter < 1024 && result == GSL_CONTINUE; ++iter) {
      result = gsl_multifit_fdfsolver_iterate(solver);
      if(result) break;
      result = gsl_multifit_test_delta(solver->dx, solver->x, 0.0001, 0.0001);
    }

	// move result to curve
	if(result == GSL_SUCCESS || result == GSL_CONTINUE) for(t_int i = 0; i < curveTypes[type].nParameters; ++i) c.parameters[i] = gsl_vector_get(solver->x, i);
	else c.type = 0;

	// clean up
	gsl_vector_free(x);
	gsl_multifit_fdfsolver_free(solver);

	return c;
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsView::clear(void) {
	data.clear();
	curve.type = 0;
}
