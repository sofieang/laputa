#include "Parameters.h"
#include "Trust.h"

const char* PolicyStr(t_int v) {
	if(v == KEEP_CONSTANT) return "constant";
	else if(v == VARY_GLOBALLY) return "default";
	else return "variable";
}

t_int PolicyValue(const char* c) {
	if(c == 0) return VARY_GLOBALLY;
	if(strcmp(c, "constant") == 0) return KEEP_CONSTANT;
	else if(strcmp(c, "default") == 0) return VARY_GLOBALLY;
	else return VARY_INDIVIDUALLY;
}

const char* EvidenceStr(t_int v) {
	if(v == NEW_EVIDENCE_NONE) return "none";
	else if(v == NEW_EVIDENCE_INQUIRY) return "inquiry";
	else return "any";
}

t_int EvidenceValue(const char* c) {
	if(c == 0) return NEW_EVIDENCE_NONE;
	if(strcmp(c, "none") == 0) return NEW_EVIDENCE_NONE;
	else if(strcmp(c, "inquiry") == 0) return NEW_EVIDENCE_INQUIRY;
	else return NEW_EVIDENCE_ANY;
}

//-----------------------------------------------------------------------------------------------------------------------

InquirerParameters::InquirerParameters(void) {
	varyStartBelief = varyInquiryChance = varyInquiryAccuracy = varyInquiryTrust = VARY_GLOBALLY;
}

//-----------------------------------------------------------------------------------------------------------------------

InquirerParameters::InquirerParameters(const InquirerParameters& ip1, const InquirerParameters& ip2, t_float v) {
	// copy discrete values
	if(v >= 0.5) *this = ip2;
	else *this = ip1;
	
	// interpolate distributions
	startBelief = Distribution(ip1.startBelief, ip2.startBelief, v);
	inquiryChance = Distribution(ip1.inquiryChance, ip2.inquiryChance, v);
	inquiryAccuracy = Distribution(ip1.inquiryAccuracy, ip2.inquiryAccuracy, v);
	inquiryTrust = MetaDistribution(ip1.inquiryTrust, ip2.inquiryTrust, v);
	
}

//-----------------------------------------------------------------------------------------------------------------------

void InquirerParameters::setDefault(void) {
	startBelief = inquiryChance = inquiryAccuracy = Distribution();
	inquiryTrust.setDefault();
	inquiryTrust.zero.setResolution(TRUST_FUNCTION_RESOLUTION + 1);
	inquiryTrust.one.setResolution(TRUST_FUNCTION_RESOLUTION + 1);
	varyStartBelief = varyInquiryChance = varyInquiryAccuracy = varyInquiryTrust = VARY_GLOBALLY;
}

//-----------------------------------------------------------------------------------------------------------------------

InquirerParameters::InquirerParameters(TiXmlElement* xml) {
	startBelief = Distribution(xml->FirstChildElement("START_BELIEF"));
	inquiryChance = Distribution(xml->FirstChildElement("INQ_CHANCE"));
	inquiryAccuracy = Distribution(xml->FirstChildElement("INQ_ACCURACY"));
	inquiryTrust = MetaDistribution(xml->FirstChildElement("INQ_TRUST"));
	varyStartBelief = PolicyValue(xml->Attribute("START_BELIEF_POLICY"));
	varyInquiryChance = PolicyValue(xml->Attribute("INQ_CHANCE_POLICY"));
	varyInquiryAccuracy = PolicyValue(xml->Attribute("INQ_ACCURACY_POLICY"));
	varyInquiryTrust = PolicyValue(xml->Attribute("INQ_TRUST_POLICY"));
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* InquirerParameters::toXML(void) const {
	// fill out structure
	TiXmlElement* xml = new TiXmlElement("INQUIRER_PARAMETERS");
	xml->LinkEndChild(startBelief.toXML("START_BELIEF"));
	xml->LinkEndChild(inquiryChance.toXML("INQ_CHANCE"));
	xml->LinkEndChild(inquiryAccuracy.toXML("INQ_ACCURACY"));
	xml->LinkEndChild(inquiryTrust.toXML("INQ_TRUST"));
	xml->SetAttribute("START_BELIEF_POLICY", PolicyStr(varyStartBelief));
	xml->SetAttribute("INQ_CHANCE_POLICY", PolicyStr(varyInquiryChance));
	xml->SetAttribute("INQ_ACCURACY_POLICY", PolicyStr(varyInquiryAccuracy));
	xml->SetAttribute("INQ_TRUST_POLICY", PolicyStr(varyInquiryTrust));
			
	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

string InquirerParameters::getDescription(void) {
	string str("");
	str += string("Inquirer starting belief distribution: ") + startBelief.getDescription() + string("\r\n");
	str += string("Inquiry chance distribution: ") + inquiryChance.getDescription() + string("\r\n");
	str += string("Inquiry accuracy distribution: ") + inquiryAccuracy.getDescription() + string("\r\n");
	str += string("Inquiry trust metadistribution: ") + inquiryTrust.getDescription();

	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

string InquirerParameters::compareTo(const InquirerParameters& ip) {
	string s("");
	
	return s;
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const InquirerParameters& lhs, const InquirerParameters& rhs) {
	if (lhs.startBelief != rhs.startBelief) return false;
	if (lhs.inquiryChance != rhs.inquiryChance) return false;
	if (lhs.inquiryAccuracy != rhs.inquiryAccuracy) return false;
	if (lhs.inquiryTrust != rhs.inquiryTrust) return false;
	if (lhs.varyStartBelief != rhs.varyStartBelief) return false;
	if (lhs.varyInquiryChance != rhs.varyInquiryChance) return false;
	if (lhs.varyInquiryAccuracy != rhs.varyInquiryAccuracy) return false;
	if (lhs.varyInquiryTrust != rhs.varyInquiryTrust) return false;
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

LinkParameters::LinkParameters(void) {
	varyListenChance = varyThreshold = varyTrust = VARY_GLOBALLY;
}

//-----------------------------------------------------------------------------------------------------------------------

LinkParameters::LinkParameters(const LinkParameters& lp1, const LinkParameters& lp2, t_float v) {
	// copy discrete values
	if(v >= 0.5) *this = lp2;
	else *this = lp1;
	
	// interpolate distributions
	linkListenChance = Distribution(lp1.linkListenChance, lp2.linkListenChance, v);
	linkThreshold = Distribution(lp1.linkThreshold, lp2.linkThreshold, v);
	linkTrust = MetaDistribution(lp1.linkTrust, lp2.linkTrust, v);
	
	
}
//-----------------------------------------------------------------------------------------------------------------------

void LinkParameters::setDefault(void) {
	linkTrust.setDefault();
	linkTrust.zero.setResolution(TRUST_FUNCTION_RESOLUTION + 1);
	linkTrust.one.setResolution(TRUST_FUNCTION_RESOLUTION + 1);
	linkThreshold = defaultDistributions[DF_DISTR_INTERVAL_UPPER];
	varyListenChance = varyThreshold = varyTrust = VARY_GLOBALLY;

}

//-----------------------------------------------------------------------------------------------------------------------


LinkParameters::LinkParameters(TiXmlElement* xml) {
	linkListenChance = Distribution(xml->FirstChildElement("LISTEN_CHANCE"));
	linkThreshold = Distribution(xml->FirstChildElement("LINK_THRESHOLD"));
	linkTrust = MetaDistribution(xml->FirstChildElement("LINK_TRUST"));
	varyListenChance = PolicyValue(xml->Attribute("LISTEN_CHANCE_POLICY"));
	varyThreshold = PolicyValue(xml->Attribute("THRESHOLD_POLICY"));
	varyTrust = PolicyValue(xml->Attribute("TRUST_POLICY"));
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* LinkParameters::toXML(void) const {
	// fill out strucure
	TiXmlElement* xml = new TiXmlElement("LINK_PARAMETERS");
	xml->LinkEndChild(linkListenChance.toXML("LISTEN_CHANCE"));
	xml->LinkEndChild(linkThreshold.toXML("LINK_THRESHOLD"));
	xml->LinkEndChild(linkTrust.toXML("LINK_TRUST"));
	xml->SetAttribute("LISTEN_CHANCE_POLICY", PolicyStr(varyListenChance));
	xml->SetAttribute("THRESHOLD_POLICY", PolicyStr(varyThreshold));
	xml->SetAttribute("TRUST_POLICY", PolicyStr(varyTrust));

	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

string LinkParameters::getDescription(void) {
	string str("");
	str += string("Link listen chance distribution: ") + linkListenChance.getDescription() + string("\r\n");
	str += string("Link threshold distribution: ") + linkThreshold.getDescription() + string("\r\n");
	str += string("Link trust metadistribution: ") + linkTrust.getDescription();

	return str;
}

//-----------------------------------------------------------------------------------------------------------------------


string LinkParameters::compareTo(const LinkParameters& lp) {
	string s("");
	
	return s;
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const LinkParameters& lhs, const LinkParameters& rhs) {
	if (lhs.linkThreshold != rhs.linkThreshold) return false;
	if (lhs.linkTrust != rhs.linkTrust) return false;
	if (lhs.linkListenChance != rhs.linkListenChance) return false;
	if (lhs.varyListenChance != rhs.varyListenChance) return false;
	if (lhs.varyThreshold != rhs.varyThreshold) return false;
	if (lhs.varyTrust != rhs.varyTrust) return false;
	return true;
}