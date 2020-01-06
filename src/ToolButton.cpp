#include "ToolButton.h"
#include <FL/Fl.H>

//-----------------------------------------------------------------------------------------------------------------------

ToolButton::ToolButton(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Menu_Button(x, y, w, h, label) {
	pressed =  false;
	icons[0] = icons[1] = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void ToolButton::setIcons(const char *upPic, const char *downPic) {
	icons[0] = new Fl_PNG_Image(upPic);
	icons[1] = new Fl_PNG_Image(downPic);
}

//-----------------------------------------------------------------------------------------------------------------------

void ToolButton::setPressed(bool on) {
	pressed = on;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

bool ToolButton::isPressed(void) {
	return pressed;
}

//-----------------------------------------------------------------------------------------------------------------------

void ToolButton::draw(void) {
	if(pressed) {
		if(icons[1]) icons[1]->draw(x(), y(), w(), h());
	}
	else {
		if(icons[0]) icons[0]->draw(x(), y(), w(), h());
	}
}

//-----------------------------------------------------------------------------------------------------------------------

t_int ToolButton::handle(t_int evt) {
	switch(evt) {
	case FL_PUSH:
		// which button is pressed?
		if(Fl::event_button() == FL_LEFT_MOUSE) {
			if(!pressed) {
				do_callback();
				redraw();
				return 1;
			}
		}
		else {
			// show popup menu
			do_callback();
			redraw();
			const Fl_Menu_Item* item = menu()->popup(x() + w(), y());
			if(item) if(item->callback()) item->do_callback(this);
			return 1;
		}
		break;
	case FL_DRAG:
		if(Fl::event_x() >= x() + w() && Fl::event_y() >= y() + h()) {
			// show popup menu
			do_callback();
			redraw();
			const Fl_Menu_Item* item = menu()->popup(x() + w(), y());
			if(item) if(item->callback()) item->do_callback(this);
		}
		return 1;
	}
	return 0;
}