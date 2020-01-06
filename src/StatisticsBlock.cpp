#include "StatisticsBlock.h"

#include <cstdlib>
#include <set>
#include "Utility.h"



//-----------------------------------------------------------------------------------------------------------------------

float* StatisticsBlock::allocate(t_int w, t_int h, t_int d) {
    free();
	dim = 3;
	width = w;
	height = h;
	depth = d;
	data = new float[w * h * d];
	refcount = new t_int;
	*refcount = 1;
	xOffset = yOffset = zOffset = 0;

	// return data, to facilitate checking if allocation worked
	return data;
}


//-----------------------------------------------------------------------------------------------------------------------

void StatisticsBlock::clear(void) {
	for(t_int i = 0; i < width * height * depth; ++i) data[i] = QNAN;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::copy(void) {
	StatisticsBlock b(*this);

	if(data != nullptr) {
		--*refcount;
		b.refcount = new t_int;
		*b.refcount = 1;
		b.data = new float[width * height * depth];
		memcpy(b.data, data, sizeof(float) * width * height * depth);
	}
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsBlock::free(void) {
	if(refcount != nullptr) {
		--*refcount;
		if(*refcount <= 0) {
			delete [] data; 
			delete refcount;
		}
		data = 0;
		refcount = nullptr;
	}	
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::average(int dimension) const {
	assert(data);
	StatisticsBlock b;
	t_int w, h;
	if(dim == 1) {
		b.allocate(1);
		b.v(0) = average();
	}
	else if(dim == 2) {
		if(dimension == DIM_X) {
			b.allocate(height);
			for(t_int j = 0; j < height; ++j) {
				t_float avg = 0;
				t_int n = 0;
				for(t_int i = 0; i < width; ++i) {
					if(ISAN(data[i * width + j]))  {
						avg += data[i * width + j];
						++n;
					}
				}
				b.v(j) = avg / n;
			}
		}
		else if(dimension == DIM_Y) {
			b.allocate(width);
			for(t_int j = 0; j < width; ++j) {
				t_float avg = 0;
				t_int n = 0;
				for(t_int i = 0; i < height; ++i) {
					if(ISAN(data[i * width + j])) {
						avg += data[i * width + j];
						++n;
					}
				}
				b.v(j) = avg / n;
			}
		}
	}
	else if(dim == 3) {
		t_int d, kStride, jStride, iStride;
		if (dimension == DIM_X) {
			w = height;
			h = depth;
			d = width;
			iStride = 1;
			jStride = width;
			kStride = height * width;
		}
		else if (dimension == DIM_Y) {
			w = width;
			h = depth;
			d = height;
			iStride = width;
			jStride = 1;
			kStride = height * width;
		}
		else if (dimension == DIM_Z) {
			w = width;
			h = height;
			d = depth;
			iStride = height * width;
			jStride = 1;
			kStride = width;
		}
		else assert(false);
		b.allocate(w, h);
		
		for(t_int k = 0; k < h; ++k) {
			for(t_int j = 0; j < w; ++j) {
				t_float avg = 0;
				t_int n = 0;
				for(t_int i = 0; i < d; ++i) {
					if(ISAN(data[k * kStride + j * jStride + i * iStride])) {
						avg += data[k * kStride + j * jStride + i * iStride];
						++n;
					}
				}
				b.v(j, k) = avg / n;
			}
		}
	}
	
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float StatisticsBlock::average(void) const {
	assert(data);
	t_float avg = 0;
	t_int n = 0;
	for(t_int i = 0; i < width * height * depth; ++i) {
		if(ISAN(data[i])) {
			avg += data[i];
			++n;
		}
	}
	return avg / n;
}

//-----------------------------------------------------------------------------------------------------------------------

#define N_BUCKETS 8

void StatisticsBlock::xSort(t_int start, t_int size) {
	t_int nans = 0;

	// find max & min values
	float max = NEGINF, min = POSINF;
	for(t_int i = 0; i < size; ++i) {
		if(ISAN(data[start + i])) {
			if(data[start + i] > max) max = data[start + i];
			if(data[start + i] < min) min = data[start + i];
		}
		else ++nans;
	}
	float mean = (max + min) * 0.5, w = max - mean;

	if(nans >= size - 8 || w < 0.0001) {
		// just do a simple insertion sort
		multiset<float, less<float>> vals;
		for(t_int i = 0; i < size; ++i) if(ISAN(data[start + i])) vals.insert(data[start + i]);
		t_int p = 0;
		for(multiset<float, less<float>>::iterator f = vals.begin(); f != vals.end(); ++f) {
			data[start + (p)] = *f;
			p++;
		}
		for(t_int i = 0; i < nans; ++i) {
			data[start + p] = QNAN;
			++p;
		}
	}
	else {
		// do a bucket sort
		multiset<float, less<float>> highBuckets[N_BUCKETS + 1], lowBuckets[N_BUCKETS + 1];
	
		float scale = N_BUCKETS / (w * w);
		for(t_int i = 0; i < size; ++i) {
			float v = data[start + i];
			if(ISAN(v)) {
				if(v >= mean) highBuckets[Round((v - mean) * (v - mean) * scale)].insert(v);
				else lowBuckets[Round((mean - v) * (mean - v) * scale)].insert(v);
			}
		}
	
		// enter values into block again
		t_int p = 0;
		for(t_int i = N_BUCKETS; i >= 0; --i) {
			for(multiset<float, less<float>>::iterator f = lowBuckets[i].begin(); f != lowBuckets[i].end(); ++f) data[start + (p++)] = *f;
		}
		for(t_int i = 0; i <= N_BUCKETS; ++i) {
			for(multiset<float, less<float>>::iterator f = highBuckets[i].begin(); f != highBuckets[i].end(); ++f) data[start + (p++)] = *f;
		}
		for(t_int i = 0; i < nans; ++i) data[start + (p++)] = QNAN;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsBlock::sort(t_int dimension) {
	assert(data);
	if(dimension == DIM_X) {
		for(t_int k = 0; k < depth; ++k) {
			for(t_int j = 0; j < height; ++j) {
				xSort(k * height * width + j * width, width);
			}
		}
	}
	else if(dimension == DIM_Y) {
		// not done yet
	}
	else if(dimension == DIM_Z) {
		// not done yet
	}
}


//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::extract(t_int xStart, t_int xEnd, t_int yStart, t_int yEnd, t_int zStart, t_int zEnd) {
	assert(data);
	StatisticsBlock b;
	assert(xEnd > xStart && yEnd > yStart && zEnd > zStart);
	assert(dim == 3);
	t_int w = xEnd - xStart, h = yEnd - yStart, d = zEnd - zStart;
	
	b.allocate(w, h, d);
	
	for(t_int k = 0; k < d; ++k) {
		for(t_int j = 0; j < h; ++j) {
			for(t_int i = 0; i < w; ++i) b.data[k * h * w + j * w + i] = data[(k + zStart) * height * width + (j + yStart) * width + i + xStart];						
		}
	}
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::extract(t_int xStart, t_int xEnd, t_int yStart, t_int yEnd) {
	assert(data);
	StatisticsBlock b;
	assert(xEnd > xStart && yEnd > yStart);
	assert(dim == 2);
	t_int w = xEnd - xStart, h = yEnd - yStart;
	b.allocate(w, h);
	
	for(t_int j = 0; j < h; ++j) for(t_int i = 0; i < w; ++i) b.data[j * w + i] = data[(j + yStart) * width + i + xStart];						

	return b;
}

			
//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::extract(t_int xStart, t_int xEnd) {
	assert(data);
	StatisticsBlock b;
	assert(xEnd > xStart);
	assert(dim == 1);
	t_int w = xEnd - xStart;
	b.allocate(w);
	
	for(t_int i = 0; i < w; ++i) b.data[i] = data[i + xStart];						
	
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::extract(t_int dimension, t_int start, t_int end) {
	assert(data);
	if(dimension == DIM_X) {
		if(dim == 1) return extract(start, end);
		else if(dim == 2) return extract(start, end, 0, height);
		else return extract(start, end, 0, height, 0, depth);
		
	}
	else if(dimension == DIM_Y) {
		assert(dim >= 2);
		if(dim == 2) return extract(0, width, start, end);
		else return extract(0, width, start, end, 0, depth);
	}
	else {
		assert(dim == 3);
		return extract(0, width, 0, height, start, end);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::extractByPercentile(t_int dimension, t_float startpercentile, t_float endpercentile) {
	StatisticsBlock b;
	if(dimension == DIM_X) {
		b.allocate(ceil(width * (endpercentile - startpercentile) / 100.0), height, depth);
		b.clear();
		for(t_int k = 0; k < depth; ++k) {
			for(t_int j = 0; j < height; ++j) {
				t_int n = 0;
				for(t_int i = 0; i < width; ++i) if(ISAN(data[k * height * width + j * width + i])) ++n;
				for(t_int i = startpercentile * n / 100, i2 = 0; i < endpercentile * n / 100; ++i, ++i2) b.v(i2, j, k) = v(i, j, k);
			}
		}
	}
	else if(dimension == DIM_Y) {
		assert(false);
		// not done yet
	}
	else if(dimension == DIM_Z) {
		assert(false);
		// not done yet
	}
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::partialYAverage(t_float yStart, t_float yEnd, bool includeStart, bool includeEnd) {
	StatisticsBlock b;
	b.allocate(height);

	for (t_int k = 0; k < height; ++k) {
		// calculate average
		t_int n = 0;
		t_float avg = 0;
		for (t_int j = 0; j < depth; ++j) {
			for (t_int i = 0; i < width; ++i) {
				t_float val = data[j * height * width + k * width + i];
				if (ISAN(val)) {
					if ((val > yStart || (val == yStart && includeStart)) && (val < yEnd || (val == yEnd && includeEnd))) {
						++n;
						avg += val;
					}
				}
			}
		}
		if (n == 0) b.v(k) = NAN;
		else b.v(k) = avg / n;
	}
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::permute(t_int xAxis, t_int yAxis, t_int zAxis) {
	t_int w, h, d, iStride, jStride, kStride;
	if (xAxis == DIM_X) {
		w = width;
		iStride = 1;
	}
	else if (xAxis == DIM_Y) {
		w = height;
		iStride = width;
	}
	else if (xAxis == DIM_Z) {
		w = depth;
		iStride = width * height;
	}
	else assert(false);
	if (yAxis == DIM_X) {
		h = width;
		jStride = 1;
	}
	else if (yAxis == DIM_Y) {
		h = height;
		jStride = width;
	}
	else if (yAxis == DIM_Z) {
		h = depth;
		jStride = width * height;
	}
	else assert(false);
	if (zAxis == DIM_X) {
		d = width;
		kStride = 1;
	}
	else if (zAxis == DIM_Y) {
		d = height;
		kStride = width;
	}
	else if (zAxis == DIM_Z) {
		d = depth;
		kStride = width * height;
	}
	else assert(false);
	
	StatisticsBlock b;
	b.allocate(w, h, d);
	for(t_int k = 0; k < d; ++k) {
		for(t_int j = 0; j < h; ++j) {
			for(t_int i = 0; i < w; ++i) b.data[k * h * w + j * w + i] = data[k * kStride + j * jStride + i * iStride];						
		}
	}
	return b;
}

//-----------------------------------------------------------------------------------------------------------------------

StatisticsBlock StatisticsBlock::permute(t_int xAxis, t_int yAxis) {
	if(xAxis == DIM_X) return *this;
	else {
		StatisticsBlock b(height, width);
		for(t_int j = 0; j < b.height; ++j) {
			for(t_int i = 0; i < b.width; ++i) b.data[j * b.width + i] = data[i * width + j];
		}
		return b;
	}
}



