#ifndef __STATISTICSBLOCK_H__
#define __STATISTICSBLOCK_H__

#include "Prefix.h"
#include <assert.h>
#include <cstring>
#include "gsl/gsl_math.h"
#include <vector>

#define DIM_X 0
#define DIM_Y 1
#define DIM_Z 2

using namespace std;

class StatisticsBlock {
public:
	StatisticsBlock() {};
	StatisticsBlock(t_int w, t_int h = 1, t_int d = 1) {allocate(w, h, d);}
	~StatisticsBlock() {free();}

	StatisticsBlock(const StatisticsBlock& b) {
        xOffset = b.xOffset;
        yOffset = b.yOffset;
        zOffset = b.zOffset;
        width = b.width;
        height = b.height;
        depth = b.depth;
        dim = b.dim;
        data = b.data;
        refcount = b.refcount;
		if(refcount != nullptr) ++*refcount;
	}
	const StatisticsBlock& operator=(const StatisticsBlock& b) {
        if(&b == this) return *this;
        xOffset = b.xOffset;
        yOffset = b.yOffset;
        zOffset = b.zOffset;
        width = b.width;
        height = b.height;
        depth = b.depth;
        dim = b.dim;
        if(data != b.data) {
            free();
            data = b.data;
            refcount = b.refcount;
            if(refcount) ++*refcount;
        }
		return *this;
	}

	float* allocate(t_int w) {
		allocate(w, 1, 1);
		dim = 1;
		return data;
	}
	float* allocate(t_int w, t_int h) {
		allocate(w, h, 1);
		dim = 2;
		return data;
	}
	float* allocate(t_int w, t_int h, t_int d);
	void free(void);
	void clear(void);
	StatisticsBlock copy(void);


	float& v(t_int x) {
		assert(data != 0 && dim == 1 && x + xOffset < width);
		return data[x + xOffset];
	}
	float& v(t_int x, t_int y) {
		assert(data != 0 && dim == 2 && x + xOffset < width && y + yOffset < height);
		return data[(y + yOffset) * width + x + xOffset];
	}
	float& v(t_int x, t_int y, t_int z) {
		assert(data != 0 && dim == 3 && x + xOffset < width && y + yOffset < height && z + zOffset < depth);
		return data[(z + zOffset) * height * width + (y + yOffset) * width + x + xOffset];
	}

	const float v(t_int x) const {
		assert(data != 0 && dim == 1 &&x + xOffset < width);
		return data[x + xOffset];
	}
	const float v(t_int x, t_int y) const {
		assert(data != 0 && dim == 2 && x + xOffset < width && y + yOffset< height);
		return data[(y + yOffset) * width + x + xOffset];
	}
	const float v(t_int x, t_int y, t_int z) const {
		assert(data != 0 && dim == 3 && x + xOffset < width && y + yOffset< height && z + zOffset < depth);
		return data[(z + zOffset) * height * width + (y + yOffset) * width + x + xOffset];
	}

	void setOffset(t_int xo = 0, t_int yo = 0, t_int zo = 0) {
		xOffset = xo;
		yOffset = yo;
		zOffset = zo;
	}

	bool valid(void) {return data != 0;}
	void invalidate(void) {
		if(refcount != nullptr) --*refcount;
		data = nullptr;
		refcount = nullptr;
	}

	vector<t_float> toVector(void) const {
		assert(dim == 1);
		vector<t_float> v(width);
		for(t_int i = 0; i < width; ++i) v[i] = data[i];
		return v;
	}

	void setDimension(t_int d) {assert(d >= dim); dim = d;}

	StatisticsBlock average(int dimension) const;
	t_float average(void) const;
	StatisticsBlock partialYAverage(t_float yStart, t_float yEnd, bool includeStart, bool includeEnd);
	StatisticsBlock extractByPercentile(t_int dimension, t_float startpercentile, t_float endpercentile);
	StatisticsBlock extract(t_int xStart, t_int xEnd, t_int yStart, t_int yEnd, t_int zStart, t_int zEnd);
	StatisticsBlock extract(t_int xStart, t_int xEnd, t_int yStart, t_int yEnd);
	StatisticsBlock extract(t_int xStart, t_int xEnd);
	StatisticsBlock extract(t_int dimension, t_int start, t_int end);
	void sort(t_int dimension);
	void xSort(t_int start, t_int size);
	StatisticsBlock permute(t_int xAxis, t_int yAxis);
	StatisticsBlock permute(t_int xAxis, t_int yAxis, t_int zAxis);

	t_int xOffset = 0, yOffset = 0, zOffset = 0;
	t_int width = 0, height = 0, depth = 0, dim = 0;

private:
	float *data = nullptr;
	t_int *refcount = nullptr;
};


#endif
