#include "Inspectors.h"
#include "UserInterfaceItems.h"

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::configure(void) {
	// update inq. inspector
	if(societyWindow->view->selectedInquirers.size()) {
		t_int nSources = 0, nTargets = 0;
		// get mean values
		t_float belief = 0, inqChance = 0, inqAccuracy = 0;
		bool nameDiffers = false;
		set<t_int>::iterator inq(societyWindow->view->selectedInquirers.begin());
		bool updateInquiryTrust = false, inStatistics = false, isTemplate = false;
		char name[MAX_INQUIRER_NAME_LENGTH];
		strcpy(name, curSociety->people[*inq].name);
		while(inq != societyWindow->view->selectedInquirers.end()) {
			// add upp values & record discrepancies
			belief += curSociety->people[*inq].belief.v();
			inqChance += curSociety->people[*inq].inquiryChance;
			inqAccuracy += curSociety->people[*inq].inquiryAccuracy;
			if(curSociety->people[*inq].updateInquiryTrust) updateInquiryTrust = true;
			if(curSociety->people[*inq].includeInStatistics) inStatistics = true;
			if(curSociety->people[*inq].inqParams) isTemplate = true;
			if(strcmp(name, curSociety->people[*inq].name) != 0) nameDiffers = true;
			nSources += curSociety->people[*inq].nSources;
			nTargets += curSociety->people[*inq].nListeners;
			++inq;
		}
		activate();

		// fill out inquirer inspector & activate fields
		if(nameDiffers) inputName->value("");
		else inputName->value(name);
		sliderBelief->value(belief / societyWindow->view->selectedInquirers.size());
		sliderInquiryChance->value(inqChance / societyWindow->view->selectedInquirers.size());
		sliderVeracityChance->value(inqAccuracy / societyWindow->view->selectedInquirers.size());
		viewTrust->removeAllTrustFunctions();
		inq = societyWindow->view->selectedInquirers.begin();
		while(inq != societyWindow->view->selectedInquirers.end()) {
			viewTrust->setTrustFunction(&curSociety->people[*inq].inquiryTrust);
			++inq;
		}
		viewTrust->activate();
		buttonUpdateTrust->value(updateInquiryTrust);
		buttonUpdateTrust->activate();
		buttonIsTemplate->value(isTemplate);
		buttonIsTemplate->activate();
		if(isTemplate) buttonSetParameters->activate();
		else buttonSetParameters->deactivate();
		buttonIncludeInStatistics->value(inStatistics);
		buttonIncludeInStatistics->activate();
		outputSources->value(nSources);
		outputListeners->value(nTargets);
		outputSources->activate();
		outputListeners->activate();

		if(societyWindow->view->selectedInquirers.size() == 1) {
			char s[64];
			sprintf(s, "Inquirer %d", *societyWindow->view->selectedInquirers.begin() + 1);
			label(s);
		}
		else {
			label("Inquirers");
		}

		// defocus name field
		societyWindow->view->take_focus();
	}
	else {
		// deactivate window
		inputName->value("");
		viewTrust->deactivate();
		buttonUpdateTrust->deactivate();
		deactivate();
		label("Inquirer");
	}

}

//-----------------------------------------------------------------------------------------------------------------------


void InquirerWindow::setSelectedInquirerNames(const char* name) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) strcpy(curSociety->people[*inq].name, name);
	societyWindow->view->redraw();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerBeliefs(t_float belief) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) curSociety->people[*inq].belief = belief;
	societyWindow->view->redraw();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerInquiryChances(t_float inqChance) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) curSociety->people[*inq].inquiryChance = inqChance;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------


void InquirerWindow::setSelectedInquirerInquiryAccuracies(t_float inqAccuracy) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) curSociety->people[*inq].inquiryAccuracy = inqAccuracy;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerTrustFromDistribution(const Distribution& d) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) d.toTrustFunction(&curSociety->people[*inq].inquiryTrust);
	configure();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerUpdateInquiryTrust(bool update) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) curSociety->people[*inq].updateInquiryTrust = update;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerIncludeInStatistics(bool incl) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) curSociety->people[*inq].includeInStatistics = incl;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------
void InquirerWindow::setSelectedInquirerIsTemplate(bool isTempl) {
	if(isTempl) {
        for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) {
            if(!curSociety->people[*inq].inqParams) curSociety->people[*inq].inqParams = new InquirerParameters;
        }
    }
    else {
        for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) {
            if(curSociety->people[*inq].inqParams) {
                delete curSociety->people[*inq].inqParams;
                curSociety->people[*inq].inqParams = 0;
            }
        }
    }
	configure();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerWindow::setSelectedInquirerParameters(const InquirerParameters& params) {
	for(set<t_int>::iterator inq = societyWindow->view->selectedInquirers.begin(); inq != societyWindow->view->selectedInquirers.end(); ++inq) {
		if(!curSociety->people[*inq].inqParams) curSociety->people[*inq].inqParams = new InquirerParameters(params);
		else *(curSociety->people[*inq].inqParams) = params;
	}
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::configure(void) {
	// update link inspector
	if(societyWindow->view->selectedLinks.size()) {
		activate();

		// get mean values
		t_float listenChance = 0, threshold = 0;
		set<pair<t_int, t_int> >::iterator link(societyWindow->view->selectedLinks.begin());
		bool updateTrust = false, countPriorAsEvidence = false, isTemplate = false;
		t_int evidencePolicy = curSociety->getLink(link->first, link->second)->evidencePolicy;
		while(link != societyWindow->view->selectedLinks.end()) {
			// add upp values & record discrepancies
			listenChance += curSociety->getLink(link->first, link->second)->listenChance;
			threshold += curSociety->getLink(link->first, link->second)->threshold;
			if(curSociety->getLink(link->first, link->second)->updateTrust) updateTrust = true;
			if(curSociety->getLink(link->first, link->second)->countPriorAsEvidence) countPriorAsEvidence = true;
			if(curSociety->getLink(link->first, link->second)->evidencePolicy != evidencePolicy) evidencePolicy = -1;
			if(curSociety->getLink(link->first, link->second)->linkParams) isTemplate = true;
			++link;
		}

		// fill out link inspector fields
		sliderListenChance->value(listenChance / societyWindow->view->selectedLinks.size());
		sliderThreshold->value(threshold / societyWindow->view->selectedLinks.size());
		viewTrust->removeAllTrustFunctions();
		link = societyWindow->view->selectedLinks.begin();
		while(link != societyWindow->view->selectedLinks.end()) {
			linkWindow->viewTrust->setTrustFunction(&curSociety->getLink(link->first, link->second)->trust);
			++link;
		}
		viewTrust->activate();

		for(t_int i = 0; i < 3; ++i) {
			buttonEvidencePolicy[i]->activate();
			if(i == evidencePolicy) buttonEvidencePolicy[i]->value(1);
			else buttonEvidencePolicy[i]->value(0);
		}
		buttonUpdateTrust->activate();
		buttonUpdateTrust->value(updateTrust);
		buttonExcludePrior->value(countPriorAsEvidence);
		buttonIsTemplate->value(isTemplate);
		buttonIsTemplate->activate();
		if(isTemplate) buttonSetParameters->activate();
		else buttonSetParameters->deactivate();
	}
	else {
		// deactivate window
		viewTrust->deactivate();
		buttonUpdateTrust->value(0);
		buttonUpdateTrust->deactivate();
		for(t_int i = 0; i < 3; ++i) {
			buttonEvidencePolicy[i]->value(0);
			buttonEvidencePolicy[i]->deactivate();
		}
		deactivate();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkListenChances(t_float listenChance) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->listenChance = listenChance;
	redraw();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkThresholds(t_float threshold) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->threshold = threshold;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkTrustFromPreset(t_int presetValue) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->trust.setFromPreset(presetValue);
	configure();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkTrustFromDistribution(const Distribution& d) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) d.toTrustFunction(&curSociety->getLink(link->first, link->second)->trust);
	configure();
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkUpdateTrust(bool update) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->updateTrust = update;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkParameters(const LinkParameters& params) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) {
		if(!curSociety->getLink(link->first, link->second)->linkParams) curSociety->getLink(link->first, link->second)->linkParams = new LinkParameters(params);
		else *(curSociety->getLink(link->first, link->second)->linkParams) = params;
	}
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkNewEvidenceReq(t_int policy) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->evidencePolicy = policy;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkExcludePrior(bool excl) {
	for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link) curSociety->getLink(link->first, link->second)->countPriorAsEvidence = excl;
	app->touchFile();
}

//-----------------------------------------------------------------------------------------------------------------------

void LinkWindow::setSelectedLinkIsTemplate(bool isTempl) {
	if(isTempl) {
		for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link)
			if(!curSociety->getLink(link->first, link->second)->linkParams) curSociety->getLink(link->first, link->second)->linkParams = new LinkParameters;
	}
	else {
		for(set<pair<t_int, t_int> >::iterator link = societyWindow->view->selectedLinks.begin(); link != societyWindow->view->selectedLinks.end(); ++link)
			if(curSociety->getLink(link->first, link->second)->linkParams) {
				delete curSociety->getLink(link->first, link->second)->linkParams;
				curSociety->getLink(link->first, link->second)->linkParams = NULL;

			}
	}
	configure();
	app->touchFile();
}
