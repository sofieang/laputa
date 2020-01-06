#ifndef __COORDINATESELECTOR_H__
#define __COORDINATESELECTOR_H__
#include "Prefix.h"

#include <FL/Fl_Value_Input.H>
#include <FL/Fl_PNG_Image.H>


// Widget used to select a coordinate in a

class CoordinateSelector : public Fl_Valuator {
public:
	CoordinateSelector(t_int x, t_int y, t_int w, t_int h, const char *label = 0) : Fl_Valuator(x, y, w, h, label) {
		xVal = new Fl_Value_Input(x, y, w, h);
		xVal->hide();
		yVal = new Fl_Value_Input(x, y, w, h);
		yVal->hide();
		dim = 2;
	}
	~CoordinateSelector() {
		delete xVal;
		delete yVal;
	}

	void setDimensions(t_int d) {dim = d;}
	void draw(void);
	t_int handle(t_int evt);

	// valuators for x and y
	Fl_Value_Input *xVal, *yVal;

private:
	t_int dim;
};


#endif
