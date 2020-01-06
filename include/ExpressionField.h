#ifndef __EXPRESSIONFIELD_H__
#define __EXPRESSIONFIELD_H__

#include "Prefix.h"
#include <FL/Fl_Text_Editor.H>
#include <muparserx/mpParser.h>

using namespace std;

class ExpressionField : public Fl_Text_Editor {
public:
	ExpressionField(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : Fl_Text_Editor(X, Y, W, H, l){
		parser = new mup::ParserX(mup::pckALL_NON_COMPLEX);
	}
	~ExpressionField() {delete parser;}

	t_float evaluate(t_float t);
	bool validateFormula(void);
	const char* errorMessage(void);

	int handle(int evt);

	string prefix;
	mup::ParserX *parser;

};

#endif
