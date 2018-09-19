#pragma once
#include "FEMaterial.h"
#include "MathObject.h"
#include "FESolidDomain.h"
#include "FEDomainMap.h"

//---------------------------------------------------------------------------------------
// Base class for evaluating model parameters
class FEValuator
{
public:
	FEValuator() {}
	virtual ~FEValuator() {}

	virtual double eval(const FEMaterialPoint& pt) = 0;
};

//---------------------------------------------------------------------------------------
class FEConstValue : public FEValuator
{
public:
	FEConstValue(double v = 0.0) : m_val(v) {};
	double eval(const FEMaterialPoint& pt) override { return m_val; }

private:
	double	m_val;
};

//---------------------------------------------------------------------------------------
class FEMathExpression : public FEValuator
{
public:
	FEMathExpression(const std::string& s);
	~FEMathExpression();
	double eval(const FEMaterialPoint& pt) override;

private:
	std::string			m_expr;
	MSimpleExpression	m_math;
};

//---------------------------------------------------------------------------------------
class FEMappedValue : public FEValuator
{
public:
	FEMappedValue(FEDomain* dom, FEDataMap* val);

	double eval(const FEMaterialPoint& pt) override;

private:
	FEDomain*		m_dom;
	FEDataMap*		m_val;
};

//---------------------------------------------------------------------------------------
// This class represents a model parameter.
// NOTE: Work in progress!
class FEModelParam
{
public:
	FEModelParam();

	// set the value
	void setValue(double v);

	// set the valuator
	void setValuator(FEValuator* val);

	// set the scale factor
	void setScaleFactor(double s) { m_scl = s; }

	// evaluate the parameter at a material point
	double eval(const FEMaterialPoint& pt) { return m_scl*m_val->eval(pt); }

	// set the domain
	void setDomain(FEDomain* dom);

	// get the domain
	FEDomain* getDomain();

private:
	double		m_scl;	//!< scale factor. This represents the load curve value
	FEValuator*	m_val;
	FEDomain*	m_dom;	//!< domain on which this model parameter is defined (can be null if don't care)
};
