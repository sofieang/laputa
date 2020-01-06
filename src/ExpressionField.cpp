#include "ExpressionField.h"
#include "App.h"
#include "Utility.h"
using namespace mup;

//-----------------------------------------------------------------------------------------------------------------------

t_float ExpressionField::evaluate(t_float t) {
	Value v(t);
	Variable var(&v);
	parser->DefineVar("t", var);
	return parser->Eval().GetFloat();
}

//-----------------------------------------------------------------------------------------------------------------------

bool ExpressionField::validateFormula(void) {
	parser->SetExpr(buffer()->text());
	bool success = true;
	Value v(1.0);
	Variable var(&v);
	parser->DefineVar("t", var);

	try {
		parser->Eval();
	}
	catch (ParserError &e) {
		success = false;
	}
	return success;
}

//-----------------------------------------------------------------------------------------------------------------------

const char* ExpressionField::errorMessage(void) {
	static char errStr[1024];
	errStr[0] = 0;
	Value v(1.0);
	Variable var(&v);
	parser->DefineVar("t", var);

	try {
		parser->Eval();
	}
	catch (ParserError &e) {
		strcat(errStr, "Message: ");
		strcat(errStr, e.GetMsg().c_str());
		strcat(errStr, "\nFormula: ");
		strcat(errStr, e.GetExpr().c_str());
		strcat(errStr, "\nToken: ");
		strcat(errStr, e.GetToken().c_str());
		strcat(errStr, "\nPosition: ");
		strcat(errStr, IntToString(e.GetPos()));
	}
	return errStr;
}

//-----------------------------------------------------------------------------------------------------------------------

int ExpressionField::handle(int evt) {
	if(evt == FL_KEYDOWN) {
		if(Fl::event_key() == FL_Enter || Fl::event_key() == '\r') return 0;
	}

	return Fl_Text_Editor::handle(evt);
}

//-----------------------------------------------------------------------------------------------------------------------
