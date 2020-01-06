#ifndef __MULTIBATCH_H__
#define __MULTIBATCH_H__

#include "Prefix.h"
#include "BatchSimulation.h"
#include <FL/filename.H>

// batch names in a multibatch
#define BATCH_A 0
#define BATCH_B 1
#define BATCH_C 2
#define BATCH_D 3
#define BATCH_FREESTANDING -1
#define N_MULTIBATCH_VALUES 4


class MultiBatch {
public:
	MultiBatch(void);
	MultiBatch(TiXmlElement* xml);
	void process(void);
	void saveStatisticsToFile(void);
	void setDefault();
	TiXmlElement*toXML(const char* name = 0);

	// generate a batch simulation with the requested parameters, stored in curBatch
	void generateBatch(t_float x, t_float y = 0);

	// find which variables are changing
	void recordVariables(t_int whichBatch);
	void recordBatchStatistics(void);

	// batch simulation variables
	t_int stepsAtoB, stepsAtoC;
	BatchSimulation batches[4];

    // current variables
	BatchSimulation curBatch;
	t_int xStep, yStep;
	t_float *values;
	string *titles;
	Society *templateSociety;

	// which file to save result in?
	char filename[FL_PATH_MAX];
};

void MultiBatchProcess(void* data);
void StartMultiBatchSimulation(MultiBatch* mb);

#endif
