#include "Simulation.h"
#include "App.h"
#include "UserInterfaceItems.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <time.h>
#include "Utility.h"

#define EVALUE_BLF_P 2
#define EVALUE_BLF_P_OR_NOT_P 1
#define EVALUE_BLF_NOT_P 0

//-----------------------------------------------------------------------------------------------------------------------

Simulation::Simulation(void) : eValuesOverTime() {
	// init everything to defaults
	curStep = 0;
	logLevel = LOG_NONE;
	logMsg = nullptr;
	logMsgSize = 0;
	soc = 0;

	val.applicationMethod = APPLY_INDIVIDUALLY;
	val.majorityAmt = val.majorityPCert = val.majorityNotPCert = 0.5;
	val.eValues[0] = 0;
	val.eValues[1] = 0.5;
	val.eValues[2] = 1.0;
    val.exponent = 1.0;
	val.amtStrictlyGreater = val.blfPStrictlyGreater = val.blfNotPStrictlyLess =true;
}

//-----------------------------------------------------------------------------------------------------------------------

Simulation::Simulation(const Simulation& s1, const Simulation& s2, t_float v) {
	*this = s1;
	if(v >= 0.5) {
		val.applicationMethod = s2.val.applicationMethod;
		val.amtStrictlyGreater = s2.val.amtStrictlyGreater;
		val.blfPStrictlyGreater = s2.val.blfPStrictlyGreater;
		val.blfNotPStrictlyLess = s2.val.blfNotPStrictlyLess;
	}
	for(t_int i = 0; i < 3; ++i) val.eValues[i] = s1.val.eValues[i] * (1.0 - v) + s2.val.eValues[i] * v;
	val.majorityAmt = s1.val.majorityAmt * (1.0 - v) + s2.val.majorityAmt * v;
	val.majorityPCert = s1.val.majorityPCert * (1.0 - v) + s2.val.majorityPCert * v;
	val.majorityNotPCert = s1.val.majorityNotPCert * (1.0 - v) + s2.val.majorityNotPCert * v;
    val.exponent = s1.val.exponent * (1.0 - v) + s2.val.exponent * v;

}

//-----------------------------------------------------------------------------------------------------------------------

Simulation::Simulation(TiXmlElement* xml) {
	curStep = 0;
	logLevel = LOG_NONE;
	logMsg = nullptr;
	logMsgSize = 0;

	// read fields
	xml->QueryIntAttribute("APPLICATION_METHOD", &val.applicationMethod);
	if(strcmp(xml->Attribute("AMOUNT_STRICTLY_GREATER"), "true") == 0) val.amtStrictlyGreater = true;
	else val.amtStrictlyGreater = false;
	if(strcmp(xml->Attribute("BELIEF_P_STRICTLY_GREATER"), "true") == 0) val.blfPStrictlyGreater = true;
	else val.blfPStrictlyGreater = false;
	if(strcmp(xml->Attribute("BELIEF_NOT_P_STRICTLY_LESS"), "true") == 0) val.blfNotPStrictlyLess = true;
	else val.blfNotPStrictlyLess = false;
	xml->QueryFloatAttribute("EVALUE_BELIEF_NOT_P", &(val.eValues[0]));
	xml->QueryFloatAttribute("EVALUE_BELIEF_P_OR_NOT_P", &(val.eValues[1]));
	xml->QueryFloatAttribute("EVALUE_BELIEF_P", &(val.eValues[2]));
	xml->QueryFloatAttribute("MAJORITY_AMOUNT", &val.majorityAmt);
	xml->QueryFloatAttribute("P_CERTAINTY_REQUIRED", &val.majorityPCert);
	xml->QueryFloatAttribute("NOT_P_CERTAINTY_REQUIRED", &val.majorityNotPCert);
    if(xml->QueryFloatAttribute("EXPONENT", &val.exponent) != TIXML_SUCCESS) val.exponent = 1.0;

}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* Simulation::toXML(void) const {
	// fill out attributes
	TiXmlElement *s = new TiXmlElement("SIMULATION");
	s->SetAttribute("APPLICATION_METHOD", val.applicationMethod);
	if(val.amtStrictlyGreater) s->SetAttribute("AMOUNT_STRICTLY_GREATER", "true");
	else s->SetAttribute("AMOUNT_STRICTLY_GREATER", "false");
	if(val.blfPStrictlyGreater) s->SetAttribute("BELIEF_P_STRICTLY_GREATER", "true");
	else s->SetAttribute("BELIEF_P_STRICTLY_GREATER", "false");
	if(val.blfNotPStrictlyLess) s->SetAttribute("BELIEF_NOT_P_STRICTLY_LESS", "true");
	else s->SetAttribute("BELIEF_NOT_P_STRICTLY_LESS", "false");
	s->SetDoubleAttribute("EVALUE_BELIEF_NOT_P", val.eValues[0]);
	s->SetDoubleAttribute("EVALUE_BELIEF_P_OR_NOT_P", val.eValues[1]);
	s->SetDoubleAttribute("EVALUE_BELIEF_P", val.eValues[2]);
	s->SetDoubleAttribute("MAJORITY_AMOUNT", val.majorityAmt);
	s->SetDoubleAttribute("P_CERTAINTY_REQUIRED", val.majorityPCert);
	s->SetDoubleAttribute("NOT_P_CERTAINTY_REQUIRED", val.majorityNotPCert);
	s->SetDoubleAttribute("EXPONENT", val.exponent);
	return s;
}

//-----------------------------------------------------------------------------------------------------------------------

string Simulation::getDescription(void) {
	string str("");
	str += string("E-value application method: ");
	if(val.applicationMethod == APPLY_TO_MAJORITY) {
		str += string("to majority (> ") + string(DoubleToString(val.majorityAmt * 100, 1)) + string("%)\r\n");
		str += string("Certainty required for voting p: ") + string(DoubleToString(val.majorityPCert)) + string("\r\n");
		str += string("Certainty required for voting not-p: ") + string(DoubleToString(val.majorityNotPCert)) + string("\r\n");
	}
	else if(val.applicationMethod == APPLY_TO_AVERAGE) str += string("to average belief\r\n");
	else str += string("to individual beliefs\r\n");
	str += string("E-values: [p = ") + string(DoubleToString(val.eValues[EVALUE_BLF_P])) + ", ";
	str += string("p V -p = ") + string(DoubleToString(val.eValues[EVALUE_BLF_P_OR_NOT_P])) + ", ";
	str += string("-p = ") + string(DoubleToString(val.eValues[EVALUE_BLF_NOT_P])) + ", ";
    str += string("Exponent = ") + string(DoubleToString(val.exponent)) + "]\r\n";

	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::reset(void) {
    // attach to current society
    soc = curSociety;

	// reset everyone's evidence counters & make list of inquirers to include in statistics
	for(t_int i = 0; i < soc->people.size(); ++i) soc->people[i].lastInquiryResult = -1;
	for(LinkIterator link = soc->links.begin(); link != soc->links.end(); ++link) {
		link->second.lastUsed = -1;
		if(link->second.evidencePolicy > NEW_EVIDENCE_NONE) {
			link->second.message = MSG_SAY_NOTHING;
			if(soc->people[link->second.source].belief.v() > 0.5 && soc->people[link->second.source].belief.v() > link->second.threshold) link->second.message = MSG_SAY_P;
			else if(soc->people[link->second.source].belief.v() < 0.5 && soc->people[link->second.source].belief.v() < 1.0 - link->second.threshold) link->second.message = MSG_SAY_NOT_P;
		}
	}


	// set variables
	curStep = 0;
	eValue = startEValue = instantEValue();
	polarisation = startPolarisation = instantPolarisation(eValue);
	eValueDelta = eValueTotal = eValueDeltaTotal = 0;
	logMsgSize = 0;

	msgSent = inqResults = 0;

	// reset bandwagon variables
	bwTowardsP = bwTowardsNotP = 0;
	inqOverriddenTowardsP = inqOverriddenTowardsNotP = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::setTime(t_int t) {
	curStep = t;
	simulationWindow->outputTime->value(IntToString(t));
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::stepTime(void) {
	++curStep;
	simulationWindow->outputTime->value(IntToString(curStep));
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::step(t_int nStepsToTake, t_int timePerEValue) {
	// take the steps
	char str[256];


	for(t_int j = 0; j < nStepsToTake; ++j) {
		if(logLevel >= LOG_STANDARD) {
            addToLog("------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
			sprintf(str, "Time: %d\n", curStep + 1);
			addToLog(str);
		}

		// update
		soc->evolve(this);

		// if this is the "official" simulation, use it to fill out the simulation window
		if(this == app->getCurSimulation()) setSimulationWindowFrom();

		// write out data in log
		if(logLevel > LOG_NONE) {
			if(logLevel == LOG_SUMMARY) {
				sprintf(str, "(%d) ", curStep + 1);
				addToLog(str);
			}
			sprintf(str, "e-value = %.3f, e-value delta = %.3f.\n",
				eValue, eValueDelta);
			addToLog(str);

			// do the actual writing out
			writeLog();
		}

		// step time
		++curStep;

		// update statistics
		if(eValuesOverTime.valid()) for(t_int i = 0; i < soc->people.size(); ++i) eValuesOverTime.v(i, curStep / timePerEValue, 0) = individualEValue(soc->people[i].belief.v());
	}

	// do we need to update selected inquirers or links?
	if(societyWindow->view->getSelectedInquirers().size()) inquirerWindow->configure();
	if(societyWindow->view->getSelectedLinks().size()) linkWindow->configure();
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Simulation::individualEValue(t_float blf) {
	if(blf >= 0.5) return powf((val.eValues[EVALUE_BLF_P] - val.eValues[EVALUE_BLF_P_OR_NOT_P]) * (blf - 0.5) * 2.0 + val.eValues[EVALUE_BLF_P_OR_NOT_P], val.exponent);
	else return powf((val.eValues[EVALUE_BLF_P_OR_NOT_P] - val.eValues[EVALUE_BLF_NOT_P]) * blf * 2.0 + val.eValues[EVALUE_BLF_NOT_P], val.exponent);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Simulation::instantEValue(void) {
	if(val.applicationMethod == APPLY_INDIVIDUALLY) {
		t_float v = 0;
		t_int n = 0;
		for(t_int i = 0; i < soc->people.size(); ++i) {
			if(soc->people[i].includeInStatistics) {
				v += individualEValue(soc->people[i].belief.v());
				++n;
			}
		}
		return v / (t_float)n;
	}
	else if(val.applicationMethod == APPLY_TO_AVERAGE) {
		t_float v = 0;
		t_int n = 0;
		for(t_int i = 0; i < soc->people.size(); ++i) {
			if(soc->people[i].includeInStatistics) {
				v += soc->people[i].belief.v();
				++n;
			}
		}
		return individualEValue(v / (t_float)n);
	}
	else {
		// calculate majority
		t_int p = 0, notp = 0, n = 0;
		for(t_int i = 0; i < soc->people.size(); ++i) {
			if(soc->people[i].includeInStatistics) {
				++n;
				if(soc->people[i].belief.v() > 0.5) {
					if(val.blfPStrictlyGreater && soc->people[i].belief.v() > val.majorityPCert) ++p;
					else if(soc->people[i].belief.v() >= val.majorityPCert) ++p;
				}
				else {
					if(val.blfNotPStrictlyLess && soc->people[i].belief.v() < val.majorityNotPCert) ++notp;
					else if(soc->people[i].belief.v() <= val.majorityNotPCert) ++notp;
				}
			}
		}

		// does a majority believe p, or not-p?
		if(val.amtStrictlyGreater) {
			if((t_float)p > (t_float)n * val.majorityAmt) return powf(val.eValues[2], val.exponent);
			else if((t_float)notp > (t_float)n * val.majorityAmt) return powf(val.eValues[0], val.exponent);
			else return powf(val.eValues[1], val.exponent);
		}
		else {
			if((t_float)p >= (t_float)n * val.majorityAmt) return powf(val.eValues[2], val.exponent);
			else if((t_float)notp >= (t_float)n * val.majorityAmt) return powf(val.eValues[0], val.exponent);
			else return powf(val.eValues[1], val.exponent);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Simulation::instantPolarisation(t_float ev) {
	t_float p = 0;
	for(t_int i = 0; i < soc->people.size(); ++i) {
		t_float v = individualEValue(soc->people[i].belief.v()) - ev;
		p += v * v;
	}
	return sqrt(p / soc->people.size());
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::addToLog(const char* msg) {
	if (logMsg == nullptr) logMsg = new char[LOG_BUFFER_SIZE];
	t_int sz = strlen(msg);
	if(sz + logMsgSize >= LOG_BUFFER_SIZE) sz = LOG_BUFFER_SIZE - logMsgSize - 1;
	memcpy(logMsg + logMsgSize, msg, sz);
	logMsgSize += sz;
	logMsg[logMsgSize] = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::writeLog(void) {
	simulationWindow->outputLog->buffer()->insert(0, logMsg);
	logMsg[0] = 0;
	logMsgSize = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void Simulation::setSimulationWindowFrom(void) {
	// set fields
	simulationWindow->outputTime->value(IntToString(curStep));
	simulationWindow->outputEValue->value(DoubleToString(eValue));
	simulationWindow->outputEValueDelta->value(DoubleToString(eValueDelta));
	simulationWindow->outputPolarisation->value(DoubleToString(instantPolarisation(eValue)));
	simulationWindow->outputPolarisationDelta->value(DoubleToString(instantPolarisation(eValue) - startPolarisation));

	// set choice menus & check box
	simulationWindow->choiceLogLevel->value(logLevel);

	// show/hide log
	if(logLevel == LOG_NONE) {
		if(simulationWindow->outputLog->visible()) {
			simulationWindow->outputLog->hide();
			simulationWindow->resize(simulationWindow->x(), simulationWindow->y(), simulationWindow->w(), 108);
		}
	}
	else {
		if(!simulationWindow->outputLog->visible()) {
			simulationWindow->outputLog->show();
			simulationWindow->resize(simulationWindow->x(), simulationWindow->y(), simulationWindow->w(), 360);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

string Simulation::compareTo(const Simulation& sim) {
	string s("");

	if(sim.val.applicationMethod != val.applicationMethod) s += string("value method: ") + string(IntToString(val.applicationMethod)) + string(" \r");
	if(sim.val.eValues[0] != val.eValues[0]) s += string("not-p value: ") + string(DoubleToString(val.eValues[0])) + string(" \r");
	if(sim.val.eValues[1] != val.eValues[1]) s += string("p-or-not-p value: ") + string(DoubleToString(val.eValues[1])) + string(" \r");
	if(sim.val.eValues[2] != val.eValues[2]) s += string("p value: ") + string(DoubleToString(val.eValues[2])) + string(" \r");
	if(sim.val.majorityAmt != val.majorityAmt) s += string("majority amt: ") + string(DoubleToString(val.majorityAmt)) + string(" \r");
	if(sim.val.majorityPCert != val.majorityPCert) s += string("majority p cert: ") + string(DoubleToString(val.majorityPCert)) + string(" \r");
	if(sim.val.majorityNotPCert != val.majorityNotPCert) s += string("majority not-p cert: ") + string(DoubleToString(val.majorityNotPCert)) + string(" \r");

	return s;
}

//-----------------------------------------------------------------------------------------------------------------------

void RunSimulation(void* data) {
	// take one or more steps
	Simulation* s = (Simulation*)data;
	if(s->logLevel == LOG_NONE) s->step(gsl_rng_uniform_int(rng, STEPS_PER_SIMULATION_STEP) + STEPS_PER_SIMULATION_STEP / 2);
	else s->step(1);
}

//-----------------------------------------------------------------------------------------------------------------------
