#include "CoordinateSelector.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

void CoordinateSelector::draw(void) {
	fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);
	t_float vx = (xVal->value() - xVal->minimum()) / (xVal->maximum() - xVal->minimum());
	fl_draw_box(FL_FLAT_BOX, (t_int)(x() + vx * (w() - 1)), y(), 1, h(), FL_RED);
	if(dim == 2) {
		t_float vy = (yVal->value() - yVal->minimum()) / (yVal->maximum() - yVal->minimum());
		fl_draw_box(FL_FLAT_BOX, x(), (t_int)(y() + vy * (h() - 1)), h(), 1, FL_RED);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

t_int CoordinateSelector::handle(t_int evt) {
	if(evt == FL_PUSH || evt == FL_DRAG) {
		t_float px = (t_float)(Fl::event_x() - x()) / (t_float)(w() - 1);
		t_float py = (t_float)(Fl::event_y() - y()) / (t_float)(h() - 1);

		if(px < 0) px = 0;
		if(px > 1) px = 1;
		if(py < 0) py = 0;
		if(py > 1) py = 1;

		xVal->value(px * (xVal->maximum() - xVal->minimum()) + xVal->minimum());
		yVal->value(py * (yVal->maximum() - yVal->minimum()) + yVal->minimum());
		do_callback();
		redraw();
		return 1;
	}

	return 0;
}
