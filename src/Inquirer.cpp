#include "Inquirer.h"
#include "App.h"

//-----------------------------------------------------------------------------------------------------------------------

Inquirer::Inquirer(t_float xPos, t_float yPos, SocietySetup *setup) {
	if(setup) {
		// init fields
		belief = setup->inqParams.startBelief.getRandomValue();
		inquiryChance = setup->inqParams.inquiryChance.getRandomValue();
		inquiryAccuracy = setup->inqParams.inquiryAccuracy.getRandomValue();
		setup->inqParams.inquiryTrust.setTrustFunctionToRandom(&inquiryTrust);
		lastInquiryResult = -1;
		message = MSG_SAY_NOTHING;
		name[0] = 0;
		includeInStatistics = setup->includeInStatistics;
		updateInquiryTrust = setup->updateInquiryTrust;

		nSources = nListeners = 0;

		// no inq params
		inqParams = NULL;
	}
	else {
		// set fields to zero
		belief = inquiryChance = inquiryAccuracy = 0;
		includeInStatistics = true;
		lastInquiryResult = -1;
		message = MSG_SAY_NOTHING;
		includeInStatistics = true;
		updateInquiryTrust = true;
		nSources = nListeners = 0;
		inqParams = NULL;
	}

	// set position
	x = xPos;
	y = yPos;

}

//-----------------------------------------------------------------------------------------------------------------------

Inquirer::Inquirer(TiXmlElement* xml) {
	strcpy(name, xml->Attribute("NAME"));
	double v;
	xml->QueryDoubleAttribute("BELIEF", &v);
	belief.set(v);
	xml->QueryFloatAttribute("INQ_CHANCE", &inquiryChance);
	xml->QueryFloatAttribute("INQ_ACCURACY", &inquiryAccuracy);
	xml->QueryIntAttribute("LAST_INQUIRY_RESULT", &lastInquiryResult);
	xml->QueryFloatAttribute("X", &x);
	xml->QueryFloatAttribute("Y", &y);
	if(xml->Attribute("INCLUDE_IN_STATISTICS")) {
		if(strcmp(xml->Attribute("INCLUDE_IN_STATISTICS"), "true") == 0) includeInStatistics = true;
		else includeInStatistics = false;
	}
	else includeInStatistics = false;
	if(xml->Attribute("UPDATE_INQUIRY_TRUST")) {
		if(strcmp(xml->Attribute("UPDATE_INQUIRY_TRUST"), "true") == 0) updateInquiryTrust = true;
		else updateInquiryTrust = false;
	}
	else updateInquiryTrust = false;

	// read trust function
	inquiryTrust = TrustFunction(xml->FirstChildElement("TRUST_FUNCTION"));

	// read parameters
	if(xml->FirstChildElement("INQUIRER_PARAMETERS"))
		inqParams = new InquirerParameters(xml->FirstChildElement("INQUIRER_PARAMETERS"));
	else inqParams = NULL;

}

//-----------------------------------------------------------------------------------------------------------------------
Inquirer::Inquirer(const Inquirer& inq) {
	inqParams = 0;
	*this = inq;
}

//-----------------------------------------------------------------------------------------------------------------------

Inquirer& Inquirer::operator=(const Inquirer& inq) {
	strcpy(name, inq.name);
	belief = inq.belief;
	newBelief = inq.newBelief;
	inquiryChance = inq.inquiryChance;
	inquiryAccuracy = inq.inquiryAccuracy;
	inquiryTrust = inq.inquiryTrust;
	lastInquiryResult = inq.lastInquiryResult;
	message = inq.message;
	if (inqParams) delete inqParams;
	if(inq.inqParams) inqParams = new InquirerParameters(*inq.inqParams);
	else inqParams = NULL;
	x = inq.x;
	y = inq.y;
	includeInStatistics = inq.includeInStatistics;
	updateInquiryTrust = inq.updateInquiryTrust;
	listeners = inq.listeners;
	nListeners = inq.nListeners;
	nSources = inq.nSources;

	return *this;
}

//-----------------------------------------------------------------------------------------------------------------------

Inquirer::~Inquirer() {
	if(inqParams) delete inqParams;
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* Inquirer::toXML(t_int id) const {
	// fill out attributes
	TiXmlElement *inq = new TiXmlElement("INQUIRER");
	inq->SetAttribute("ID", id);
	inq->SetAttribute("NAME", name);
	inq->SetDoubleAttribute("BELIEF", belief.v());
	inq->SetDoubleAttribute("INQ_CHANCE", inquiryChance);
	inq->SetDoubleAttribute("INQ_ACCURACY", inquiryAccuracy);
	inq->SetAttribute("LAST_INQUIRY_RESULT", lastInquiryResult);
	inq->SetDoubleAttribute("X", x);
	inq->SetDoubleAttribute("Y", y);
	if(includeInStatistics) inq->SetAttribute("INCLUDE_IN_STATISTICS", "true");
	else inq->SetAttribute("INCLUDE_IN_STATISTICS", "false");
	if(updateInquiryTrust) inq->SetAttribute("UPDATE_INQUIRY_TRUST", "true");
	else inq->SetAttribute("UPDATE_INQUIRY_TRUST", "false");

	// fill out trust function
	inq->LinkEndChild(inquiryTrust.toXML());

	// fill out parameters
	if(inqParams) inq->LinkEndChild(inqParams->toXML());

	return inq;
}

//-----------------------------------------------------------------------------------------------------------------------

void Inquirer::doInquiry(Simulation *sim, Society *soc, t_int index) {
	char s[256], msg[256], inqName[64];

	// start log message
	if(sim->logLevel >= LOG_STANDARD) {
		if(name[0]) sprintf(inqName, "'%s'", name);
		else sprintf(inqName, "%d", index + 1);
	}

	// starting left-hand and right-hand sides in equation
	bool informationReceived = false;
	Amount lhs = belief, rhs = belief.inverted(), inqEffect = belief;

	// does inquiry give anything?
	if(gsl_rng_uniform(rng) < inquiryChance) {
		informationReceived = true;
		lastInquiryResult = sim->curStep;
		++sim->inqResults;
		Amount expectation = inquiryTrust.expectation();
		if(gsl_rng_uniform(rng) < inquiryAccuracy) {
			// inquiry says p
			if(sim->logLevel >= LOG_STANDARD) sprintf(msg, "Inquirer %s received the result that p from inquiry", inqName);

			// update P(p)
			lhs *= expectation;
			rhs *= expectation.inverted();
			inqEffect = lhs.dividedByAdded(rhs);

			// update trust function
			if(updateInquiryTrust) {
				inquiryTrust.update(belief.v(), true);
				if(sim->logLevel == LOG_DETAILED) {
					if(inquiryTrust.expectation() > expectation.v()) sprintf(s, ", raising his/her expected trust in it from %.3f to %.3f", expectation.v(), inquiryTrust.expectation());
					else if (inquiryTrust.expectation() < expectation.v()) sprintf(s, ", lowering his/her expected trust in it from %.3f to %.3f", expectation.v(), inquiryTrust.expectation());
					else sprintf(s, ", with no effect on his/her expected trust in it");
					strcat(msg, s);
				}
			}

			// record
			message = MSG_SAY_P;
		}
		else {
			// inquiry says not-p
			if(sim->logLevel >= LOG_STANDARD) sprintf(msg, "Inquirer %s received the result that not-p from inquiry", inqName);

			// update P(p)
			lhs *= expectation.inverted();
			rhs *= expectation;
			inqEffect = lhs.dividedByAdded(rhs);

			// update trust function
			if(updateInquiryTrust) {
				inquiryTrust.update(belief.v(), false);
				if(sim->logLevel == LOG_DETAILED) {
					if(inquiryTrust.expectation() > expectation.v()) sprintf(s, ", raising his/her expected trust in it from %.3f to %.3f", expectation.v(), inquiryTrust.expectation());
					else if (inquiryTrust.expectation() < expectation.v()) sprintf(s, ", lowering his/her expected trust in it from %.3f to %.3f", expectation.v(), inquiryTrust.expectation());
					else sprintf(s, ", with no effect on his/her expected trust in it");
					strcat(msg, s);
				}
			}

			// record
			message = MSG_SAY_NOT_P;
		}
		if(sim->logLevel >= LOG_STANDARD) {
			strcat(msg, ".\n");
			app->getCurSimulation()->addToLog(msg);
		}
	}

	// does listening to others give anything?
	for(LinkIterator link = soc->links.lower_bound(LBOUND(index)); link != soc->links.upper_bound(UBOUND(index)); ++link) {
		t_int source = link->second.source;
		// what does source say?
		t_int whatToSay = MSG_SAY_NOTHING;
		if(link->second.evidencePolicy == NEW_EVIDENCE_NONE) {
			if((soc->people[source].belief.v() > 0.5 && link->second.threshold > 0.5) || (soc->people[source].belief.v() < 0.5 && link->second.threshold < 0.5)) whatToSay = MSG_SAY_P;
			else if((soc->people[source].belief.v() < 0.5 && link->second.threshold > 0.5) || (soc->people[source].belief.v() > 0.5 && link->second.threshold < 0.5)) whatToSay = MSG_SAY_NOT_P;
			else whatToSay = (gsl_rng_get(rng) & 1) ? MSG_SAY_P : MSG_SAY_NOT_P;
		}
		else {
			// does consulation of link pass the "new evidence" test?
			bool newEvidence = link->second.lastUsed < soc->people[source].lastInquiryResult;
			if(link->second.countPriorAsEvidence && link->second.lastUsed == -1) newEvidence = true;
			if(link->second.evidencePolicy == NEW_EVIDENCE_ANY && !newEvidence) {
				for(LinkIterator link2 = soc->links.lower_bound(LBOUND(source)); link2 != soc->links.upper_bound(UBOUND(source)); ++link2) {
					if(link2->second.source != index && link2->second.lastUsed > link->second.lastUsed) {
						newEvidence = true;
						break;
					}
				}
			}
			if(newEvidence) whatToSay = link->second.message;
			else whatToSay = MSG_SAY_NOTHING;
		}

		// is link being used at this time step?
		if((gsl_rng_uniform(rng) < link->second.listenChance) && (whatToSay != MSG_SAY_NOTHING)) {
			++sim->msgSent;
			Amount expectation = const_cast<TrustFunction&>(link->second.trust).expectation();
			informationReceived = true;
			link->second.lastUsed = sim->curStep;
			if(whatToSay == MSG_SAY_P) {
				// source says p
				if(sim->logLevel >= LOG_STANDARD) {
					sprintf(msg, "Inquirer %s heard that p from inquirer ", inqName);
					if(soc->people[source].name[0]) sprintf(s, "'%s'", soc->people[source].name);
					else sprintf(s, "%d", source + 1);
					strcat(msg, s);
				}

				// update P(p)
				lhs *= expectation;
				rhs *= expectation.inverted();

				// update trust function
				if(link->second.updateTrust) {
					link->second.trust.update(belief.v(), true);
					if(sim->logLevel == LOG_DETAILED) {
						if(link->second.trust.expectation() > expectation.v()) sprintf(s, ", raising his/her expected trust in the source from %.3f to %.3f", expectation.v(), link->second.trust.expectation());
						else if (link->second.trust.expectation() < expectation.v()) sprintf(s, ", lowering his/her expected trust in the source from %.3f to %.3f", expectation.v(), link->second.trust.expectation());
						else sprintf(s, ", with no effect on his/her expected trust in the source");
						strcat(msg, s);
					}
				}
			}
			else if(whatToSay == MSG_SAY_NOT_P){
				// source says not-p;
				if(sim->logLevel >= LOG_STANDARD) {
					sprintf(msg, "Inquirer %s heard that not-p from inquirer ", inqName);
					if(soc->people[source].name[0]) sprintf(s, "'%s'", soc->people[source].name);
					else sprintf(s, "%d", source + 1);
					strcat(msg, s);
				}

				// update P(p)
				lhs *= expectation.inverted();
				rhs *= expectation;

				// update trust function
				if(link->second.updateTrust) {
					link->second.trust.update(belief.v(), false);
					if(sim->logLevel == LOG_DETAILED) {
						if(link->second.trust.expectation() > expectation.v()) sprintf(s, ", raising his/her expected trust in the source from %.3f to %.3f", expectation.v(), link->second.trust.expectation());
						else if (link->second.trust.expectation() < expectation.v()) sprintf(s, ", lowering his/her expected trust in the source from %.3f to %.3f", expectation.v(), link->second.trust.expectation());
						else sprintf(s, ", with no effect on his/her expected trust in the source");
						strcat(msg, s);
					}
				}
			}
			if(sim->logLevel >= LOG_STANDARD) {
				strcat(msg, ".\n");
				app->getCurSimulation()->addToLog(msg);
			}
		}
	}

	// Calculate new belief value (left hand side / (left hand side + right hand side)) if non-contradictory
	if(informationReceived) {
		if(lhs + rhs > 0) {
			newBelief = lhs.dividedByAdded(rhs);
			if(sim->logLevel >= LOG_STANDARD) {
				if(newBelief > belief) sprintf(msg, "This raised his/her degree of belief in p from %.5f to %.5f.\n", belief.v(), newBelief.v());
				else if(newBelief < belief) sprintf(msg, "This lowered his/her degree of belief in p from %.5f to %.5f\n", belief.v(), newBelief.v());
				else sprintf(msg, "This did not affect his/her degree of belief in p.\n");
				app->getCurSimulation()->addToLog(msg);
			}

			// Note possible bandwagon effect
			if(newBelief > inqEffect) {
				sim->bwTowardsP += newBelief.v() - inqEffect.v();
				++sim->inqOverriddenTowardsP;
			}
			else if(newBelief < inqEffect) {
				sim->bwTowardsNotP += inqEffect.v() - newBelief.v();
				++sim->inqOverriddenTowardsNotP;
			}

		}
		else if(sim->logLevel > LOG_NONE) {
			sprintf(msg, "The information gathered by inquirer %s forced him/her t_into contradiction.\n", inqName);
			app->getCurSimulation()->addToLog(msg);
		}

		// Notice if inquirer becomes certain
		if(newBelief == 1.0 && belief != 1.0) {
			if(sim->logLevel >= LOG_STANDARD) {
				sprintf(msg, "Inquirer %s became certain that p.\n", inqName);
				app->getCurSimulation()->addToLog(msg);
			}
		}
		if(newBelief == 0.0 && belief != 1.0) {
			if(sim->logLevel >= LOG_STANDARD) {
				sprintf(msg, "Inquirer %s became certain that not-p.\n", inqName);
				app->getCurSimulation()->addToLog(msg);
			}
		}
	}
	else {
		// don't change belief if nothing has changed
		newBelief = belief;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const Inquirer& lhs, const Inquirer& rhs) {
	if (strcmp(lhs.name, rhs.name) != 0) return false;
	if (lhs.belief != rhs.belief) return false;
	if (lhs.newBelief != rhs.newBelief) return false;
	if (lhs.inquiryChance != rhs.inquiryChance) return false;
	if (lhs.inquiryAccuracy != rhs.inquiryAccuracy) return false;
	if (lhs.inquiryTrust != rhs.inquiryTrust) return false;
	if (lhs.lastInquiryResult != rhs.lastInquiryResult) return false;
	if (lhs.message != rhs.message) return false;
	if ((lhs.inqParams == 0) != (rhs.inqParams == 0)) return false;
	else if ((lhs.inqParams != 0) && (rhs.inqParams != 0)) if (*lhs.inqParams != *rhs.inqParams) return false;
	if (lhs.x != rhs.x) return false;
	if (lhs.y != rhs.y) return false;
	if (lhs.includeInStatistics != rhs.includeInStatistics) return false;
	if (lhs.updateInquiryTrust != rhs.updateInquiryTrust) return false;
	vector<t_int>::const_iterator lhsl = lhs.listeners.begin(), rhsl = rhs.listeners.begin();
	while (lhsl != lhs.listeners.end() && rhsl != rhs.listeners.end()) if (*(lhsl++) != *(rhsl++)) return false;
	if (lhsl != lhs.listeners.end() || rhsl != rhs.listeners.end()) return false;
	if (lhs.nListeners != rhs.nListeners) return false;
	if (lhs.nSources != rhs.nSources) return false;
	return true;
}
