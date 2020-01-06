#ifndef HEXFIELD_H
#define HEXFIELD_H
#include <FL/Fl_Input.h>
#include <cstring>

// HexField - a small class to work with hexadecimal data

class HexField : public Fl_Input {
public:
	HexField(int x, int y, int w, int h, const char *label = 0) : Fl_Input(x, y, w, h, label) {
		
	};
	
	// overloaded event handling, to filter out non-hex characters
	int handle(int evt) {
		if(evt == FL_KEYDOWN) {
			int key = Fl::event_key();
			if(	(key >= '0' && key <= '9') ||
				(key >= 'a' && key <= 'f') ||
				(key >= '0' + FL_KP && key <= '9' + FL_KP)) {
				
				if(strlen(value()) - (mark() - position()) >= 8) return 0;
				return Fl_Input::handle(evt);
			}
			else if (key == FL_Delete || key == FL_BackSpace || key == FL_Left || key == FL_Right) return Fl_Input::handle(evt);
			else return 0;
		}
		else return Fl_Input::handle(evt);
	}
	
	// translate hex to integer
	unsigned int valueNum(void) {
		return hexToNum(value());
	}
	
	// set text from integer
	void valueNum(unsigned int v)  {
		value(numToHex(v));
	}
	
	static const char* numToHex(unsigned int v) {
		static char str[9];
		int shift = 0;
		for(int i = 7; i >= 0; --i) {
			int val = (v >> shift) & 15;
			if(val <= 9) str[i] = '0' + val;
			else str[i] = 'a' + val - 10;
			shift += 4;
		}
		str[8] = 0;
		return(str);
	}
	
	static unsigned int hexToNum(const char* s) {
		unsigned int v = 0;
		int shift = 0, pos = strlen(s) - 1;
		if(pos > 7) pos = 7;
		while(pos >= 0) {
			if(s[pos] >= '0' && s[pos] <= '9') v += (s[pos] - '0') << shift;
			else if(s[pos] >= 'a' && s[pos] <= 'f') v += (s[pos] - 'a' + 10) << shift;
			else if(s[pos] >= 'A' && s[pos] <= 'f') v += (s[pos] - 'A' + 10) << shift;
			
			--pos;
			shift += 4;
		}
		return v;
	}

};


#endif