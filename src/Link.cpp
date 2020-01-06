#include "Link.h"
#include "Society.h"





//-----------------------------------------------------------------------------------------------------------------------

Link::Link(t_int from, t_int to, SocietySetup *setup) {
	if(setup) {
		// init fields
		threshold = setup->linkParams.linkThreshold.getRandomValue();
		listenChance = setup->linkParams.linkListenChance.getRandomValue();
		setup->linkParams.linkTrust.setTrustFunctionToRandom(&trust);
		evidencePolicy = setup->evidencePolicy;
		countPriorAsEvidence = setup->countPriorAsEvidence;
		updateTrust = setup->updateTrust;
		lastUsed = -1;
	}
	source = from;
	target = to;

	// null the parameters block
	linkParams = NULL;

}

//-----------------------------------------------------------------------------------------------------------------------

Link::Link(t_int from, t_int to, const Link& l) {
	*this = l;
	source = from;
	target = to;
}

//-----------------------------------------------------------------------------------------------------------------------

Link& Link::operator=(const Link& l) {
	source = l.source;
	target = l.target;
	listenChance = l.listenChance;
	trust = l.trust;
	threshold = l.threshold;
	if(l.linkParams) linkParams = new LinkParameters(*l.linkParams);
	else linkParams = NULL;
	lastUsed = l.lastUsed;
	evidencePolicy = l.evidencePolicy;
	countPriorAsEvidence = l.countPriorAsEvidence;
	updateTrust = l.updateTrust;
	return *this;
}

//-----------------------------------------------------------------------------------------------------------------------

Link::Link(TiXmlElement* xml) {
	xml->QueryIntAttribute("SOURCE", &source);
	xml->QueryIntAttribute("TARGET", &target);
	xml->QueryDoubleAttribute("LISTEN_CHANCE", &listenChance);
	xml->QueryDoubleAttribute("THRESHOLD", &threshold);
	xml->QueryIntAttribute("LAST_USED", &lastUsed);
	trust = TrustFunction(xml->FirstChildElement("TRUST_FUNCTION"));

	evidencePolicy = EvidenceValue(xml->Attribute("NEW_EVIDENCE_REQUIREMENT"));
	if(xml->Attribute("COUNT_PRIOR_AS_EVIDENCE")) {
		if(strcmp(xml->Attribute("COUNT_PRIOR_AS_EVIDENCE"), "true") == 0) countPriorAsEvidence = true;
		else countPriorAsEvidence = false;
	}
	else countPriorAsEvidence = true;
	if(strcmp(xml->Attribute("UPDATE_TRUST"), "true") == 0) updateTrust = true;
	else updateTrust = false;

	if(xml->FirstChildElement("LINK_PARAMETERS")) linkParams = new LinkParameters(xml->FirstChildElement("LINK_PARAMETERS"));
	else linkParams = NULL;

}

//-----------------------------------------------------------------------------------------------------------------------

Link::~Link() {
	if(linkParams) delete linkParams;
}

//-----------------------------------------------------------------------------------------------------------------------

void Link::mergeWithNew(SocietySetup *setup) {
	t_float lc = setup->linkParams.linkListenChance.getRandomValue(), scale = 1.0 / (listenChance + lc);
	threshold = (threshold * listenChance + setup->linkParams.linkThreshold.getRandomValue() * lc) * scale;
	setup->linkParams.linkTrust.mergeTrustFunctionWithRandom(&trust, lc * scale);
	listenChance = listenChance + lc - listenChance * lc;
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* Link::toXML(void) const {
	TiXmlElement* xml = new TiXmlElement("LINK");
	xml->SetAttribute("SOURCE", source);
	xml->SetAttribute("TARGET", target);
	xml->SetAttribute("LAST_USED", lastUsed);
	xml->SetDoubleAttribute("LISTEN_CHANCE", listenChance);
	xml->SetDoubleAttribute("THRESHOLD", threshold);
	xml->SetAttribute("NEW_EVIDENCE_REQUIREMENT", EvidenceStr(evidencePolicy));
	if(updateTrust) xml->SetAttribute("UPDATE_TRUST", "true");
	else xml->SetAttribute("UPDATE_TRUST", "false");
	if(countPriorAsEvidence) xml->SetAttribute("COUNT_PRIOR_AS_EVIDENCE", "true");
	else xml->SetAttribute("COUNT_PRIOR_AS_EVIDENCE", "false");

	// fill out trust function
	xml->LinkEndChild(trust.toXML());

	// fill out parameters
	if(linkParams) xml->LinkEndChild(linkParams->toXML());

	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const Link& lhs, const Link& rhs) {
	if (lhs.source != rhs.source) return false;
	if (lhs.target != rhs.target) return false;
	if (lhs.message != rhs.message) return false;
	if (lhs.listenChance != rhs.listenChance) return false;
	if (lhs.trust != rhs.trust) return false;
	if (lhs.threshold != rhs.threshold) return false;
	if (lhs.lastUsed != rhs.lastUsed) return false;
	if ((lhs.linkParams == 0) != (rhs.linkParams == 0)) return false;
	if (lhs.linkParams != 0 && rhs.linkParams != 0) if (*lhs.linkParams != *rhs.linkParams) return false;
	if (lhs.updateTrust != rhs.updateTrust) return false;
	if (lhs.evidencePolicy != rhs.evidencePolicy) return false;
	if (lhs.countPriorAsEvidence != rhs.countPriorAsEvidence) return false;
	return true;
}
