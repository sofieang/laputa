#ifndef __INQUIRER_H__
#define __INQUIRER_H__

#define MAX_INQUIRER_NAME_LENGTH 32
#define INQUIRER_CIRCLE_SIZE 15.0f

#include "Prefix.h"

#include <forward_list>

#include "SocietySetup.h"
#include "Link.h"
#include "Trust.h"
#include "Amount.h"
#include "tinyxml.h"


using namespace std;
class Society;
class Simulation;

// Inquirer - represents a Bayesian inquirer, and also handles saving and loading of said inquirer.

class Inquirer {
public:
	// constructor & destructor
	Inquirer(t_float xPos = 100.0, t_float yPos = 100.0, SocietySetup *setup = 0);
	Inquirer(TiXmlElement* xml);
	Inquirer(const Inquirer& inq);
	~Inquirer();
	Inquirer& operator=(const Inquirer& inq);

	// handle inquiry
	void doInquiry(Simulation* sim, Society *soc, t_int index);

	TiXmlElement* toXML(t_int id) const;

	// name
	char name[MAX_INQUIRER_NAME_LENGTH];

	// parameters
	Amount belief, newBelief;
	t_float inquiryChance;
	t_float inquiryAccuracy;
	TrustFunction inquiryTrust;
	t_int lastInquiryResult;
	t_int message;

	// simulation parameters
	InquirerParameters* inqParams;

	// where is inquirer in display?
	t_float x, y;

	// should we include inquirer in statistics?
	bool includeInStatistics;

	// should we update inquirer's trust in her inquiry?
	bool updateInquiryTrust;

	// vector of inquirers listening to this, to facilitate easier lookup
	vector<t_int> listeners;

	// counts of listeners & sources
	t_int nListeners, nSources;
};

// comparison
bool operator==(const Inquirer& lhs, const Inquirer& rhs);
inline bool operator != (const Inquirer& lhs, const Inquirer& rhs) { return !(lhs == rhs); }


#endif
