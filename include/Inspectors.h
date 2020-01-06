#ifndef __INSPECTORS_H__
#define __INSPECTORS_H__
/*
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Ask.H>
#include <FL/Fl_PNG_Image.h>
#include <FL/FL_Menu_Item.h>
#include <FL/FL_Help_Dialog.h>
#include <FL/FL_Multi_Browser.h>
#include <FL/FL_Tabs.h>
#include <FL/Fl_Choice.h>*/

#include "UserInterfaceItems.h"
#include "Distribution.h"
#include "Trust.h"
#include "Parameters.h"

class InquirerWindow;
extern InquirerWindow* inquirerWindow;
class LinkWindow;
extern LinkWindow* linkWindow;

// Inquirer inspector window, for editing inquirer attributes
class InquirerWindow : public DialogInterface {
public:
	InquirerWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {inquirerWindow = this;}
	InquirerWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {inquirerWindow = this;}
	void configure();
	
	Fl_Input *inputName = nullptr;
	Fl_Value_Slider *sliderBelief = nullptr;
	Fl_Value_Slider *sliderInquiryChance = nullptr;
	Fl_Value_Slider *sliderVeracityChance = nullptr;
	TrustView *viewTrust = nullptr;
	Fl_Button *buttonSetParameters = nullptr;
	Fl_Check_Button *buttonIsTemplate = nullptr;
	Fl_Check_Button *buttonUpdateTrust = nullptr;
	Fl_Check_Button *buttonIncludeInStatistics = nullptr;
	Fl_Value_Output *outputSources = nullptr;
	Fl_Value_Output *outputListeners = nullptr;
	
	// set properties of selected inquirers
	void setSelectedInquirerNames(const char* name);
	void setSelectedInquirerBeliefs(t_float belief);
	void setSelectedInquirerInquiryChances(t_float inqChance);
	void setSelectedInquirerInquiryAccuracies(t_float inqAccuracy);
	void setSelectedInquirerUpdateInquiryTrust(bool update);
	void setSelectedInquirerParameters(const InquirerParameters& params);
	void setSelectedInquirerIsTemplate(bool isTempl);
	void setSelectedInquirerIncludeInStatistics(bool incl);
	void setSelectedInquirerTrustFromDistribution(const Distribution& d);

	// overloaded deactivate/activate
	void deactivate(void) {
		for(t_int i = 0; i < children(); ++i) child(i)->deactivate();
		Fl_Double_Window::deactivate();
	}
	void activate(void) {
		Fl_Double_Window::activate();
		for(t_int i = 0; i < children(); ++i) child(i)->activate();
	}
};


// Link inspector window, for editing link attributes
class LinkWindow : public DialogInterface {
public:
	LinkWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {linkWindow = this;}
	LinkWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {linkWindow = this;} 
	void configure();
	
	Fl_Value_Slider *sliderListenChance = nullptr;
	Fl_Value_Slider *sliderThreshold = nullptr;
	
	TrustView *viewTrust = nullptr;
	Fl_Button *buttonSetParameters = nullptr;
	Fl_Check_Button *buttonIsTemplate = nullptr;
	Fl_Check_Button *buttonUpdateTrust = nullptr;
	
	Fl_Round_Button* buttonEvidencePolicy[3] = { nullptr, nullptr, nullptr };
	Fl_Check_Button *buttonExcludePrior = nullptr;
	
	// set properties of selected links
	void setSelectedLinkListenChances(t_float listenChance);
	void setSelectedLinkThresholds(t_float threshold);
	void setSelectedLinkUpdateTrust(bool update);
	void setSelectedLinkParameters(const LinkParameters& params);
	void setSelectedInquirerTrustFromPreset(t_int presetValue);
	void setSelectedLinkTrustFromPreset(t_int presetValue);
	void setSelectedLinkIsTemplate(bool isTempl);
	void setSelectedLinkTrustFromDistribution(const Distribution& d);
	void setSelectedLinkNewEvidenceReq(t_int policy);
	void setSelectedLinkExcludePrior(bool excl);
	
	// overloaded deactivate/activate
	void deactivate(void) {
		for(t_int i = 0; i < children(); ++i) child(i)->deactivate();
		Fl_Double_Window::deactivate();
	}
	void activate(void) {
		Fl_Double_Window::activate();
		for(t_int i = 0; i < children(); ++i) child(i)->activate();
	}
};


#endif