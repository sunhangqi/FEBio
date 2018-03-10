#pragma once
#include "FEDataArray.h"

class FENodeDataMap : public FEDataArray
{
public:
	FENodeDataMap(int dataType);

	void Create(int nsize, double val = 0.0);

	void Add(double val);

public:
	void setValue(int n, double v);
	void setValue(int n, const vec2d& v);
	void setValue(int n, const vec3d& v);

	double getValue(int n) const;

	void fillValue(double v);
	void fillValue(const vec2d& v);
	void fillValue(const vec3d& v);
};
