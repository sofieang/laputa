#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_PNG_Image.H>
#include "Prefix.h"
#include <string>
using namespace std;
// ToolButton - because I couldn't find a widget in fltk that worked exactly as I wanted it to

class ToolButton : public Fl_Menu_Button {
public:
	ToolButton(t_int x, t_int y, t_int w, t_int h, const char *label = 0);

	void setIcons(const char *upPic, const char *downPic);
	void setIcons(const string& upPic, const string& downPic) {setIcons(upPic.c_str(), downPic.c_str());}
	void setPressed(bool on);
	bool isPressed(void);

	void draw(void);
	t_int handle(t_int evt);

private:
	Fl_PNG_Image *icons[2];
	bool pressed;
};


#endif
