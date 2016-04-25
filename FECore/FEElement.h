// FEElement.h: interface for the FEElement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEELEMENT_H__2EE38101_58E2_4FEB_B214_BB71B6FB15FB__INCLUDED_)
#define AFX_FEELEMENT_H__2EE38101_58E2_4FEB_B214_BB71B6FB15FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEElementLibrary.h"
#include "FEElementTraits.h"
#include "FEMaterialPoint.h"
#include "FE_enum.h"
#include "FEException.h"

//-----------------------------------------------------------------------------
class FEElementTraits;

//-----------------------------------------------------------------------------
//! The FEElementState class stores the element state data. The state is defined
//! by a material point class for each of the integration points.
class FEElementState
{
public:
	//! default constructor
	FEElementState() {}

	//! destructor
	~FEElementState() { Clear(); }

	//! copy constructor
	FEElementState(const FEElementState& s);

	//! assignment operator
	FEElementState& operator = (const FEElementState& s);

	//! clear state data
	void Clear() { for (size_t i=0; i<m_data.size(); ++i) delete m_data[i]; m_data.clear(); }

	//! create 
	void Create(int n) { m_data.assign(n, static_cast<FEMaterialPoint*>(0) ); }

	//! operator for easy access to element data
	FEMaterialPoint*& operator [] (int n) { return m_data[n]; }

private:
	vector<FEMaterialPoint*>	m_data;
};

//-----------------------------------------------------------------------------
//! Base class for all element classes

//! From this class the different element classes are derived.

class FEElement
{
public:
	enum {MAX_NODES     = 27};	// max nr of nodes
	enum {MAX_INTPOINTS = 27};	// max nr of integration points

public:
	//! default constructor
	FEElement();

	//! destructor
	virtual ~FEElement() {}

	//! get the element ID
	int GetID() const { return m_nID; }

	//! set the element ID
	void SetID(int n) { m_nID = n; }

	//! Get the element's material ID
	int GetMatID() const { return m_mat; }

	//! Set the element's material ID
	void SetMatID(int id) { m_mat = id; }

public:
	//! Set the type of the element
	void SetType(int ntype) { FEElementLibrary::SetElementTraits(*this, ntype); }

	//! Set the traits of an element
	virtual void SetTraits(FEElementTraits* ptraits);

	//! Get the element traits
	FEElementTraits* GetTraits() { return m_pT; }

	//! return number of nodes
	int Nodes() const { return m_pT->neln; } 

	//! return the element class
	int Class() const { return m_pT->Class(); }

	//! return the element shape
	int Shape() const { return m_pT->Shape(); }

	//! return the type of element
	int Type() const { return m_pT->Type(); }

	//! return number of integration points
	int GaussPoints() const { return m_pT->nint; } 

	//! shape function values
	double* H(int n) { return m_pT->H[n]; }

public:
	//! Get the material point data
	FEMaterialPoint* GetMaterialPoint(int n) { return m_State[n]; }

	//! set the material point data
	void SetMaterialPointData(FEMaterialPoint* pmp, int n) { m_State[n] = pmp; }

public:
	//! evaluate scalar field at integration point
	double Evaluate(double* fn, int n);

	//! evaluate scale field at integration point
	double Evaluate(vector<double>& fn, int n);

	//! evaluate vector field at integration point
	vec2d Evaluate(vec2d* vn, int n);

	//! evaluate vector field at integration point
	vec3d Evaluate(vec3d* vn, int n);

protected:
	int		m_nID;		//!< element ID
	int		m_mat;		//!< material index

public:
	vector<int>		m_node;		//!< connectivity

	// This array stores the local node numbers, that is the node numbers
	// into the node list of a domain.
	vector<int>		m_lnode;	//!< local connectivity

protected:
	FEElementState		m_State;	//!< element state data
	FEElementTraits*	m_pT;		//!< pointer to element traits
};

//-----------------------------------------------------------------------------
//!  This class defines a solid element

class FESolidElement : public FEElement
{
public:
	//! default constructor
	FESolidElement(){}

	//! copy constructor
	FESolidElement(const FESolidElement& el);

	//! assignment operator
	FESolidElement& operator = (const FESolidElement& el);

	double* GaussWeights() { return &((FESolidElementTraits*)(m_pT))->gw[0]; }			// weights of integration points

	double* Gr(int n) { return ((FESolidElementTraits*)(m_pT))->Gr[n]; }	// shape function derivative to r
	double* Gs(int n) { return ((FESolidElementTraits*)(m_pT))->Gs[n]; }	// shape function derivative to s
	double* Gt(int n) { return ((FESolidElementTraits*)(m_pT))->Gt[n]; }	// shape function derivative to t

	double* Grr(int n) { return ((FESolidElementTraits*)(m_pT))->Grr[n]; }	// shape function 2nd derivative to rr
	double* Gsr(int n) { return ((FESolidElementTraits*)(m_pT))->Gsr[n]; }	// shape function 2nd derivative to sr
	double* Gtr(int n) { return ((FESolidElementTraits*)(m_pT))->Gtr[n]; }	// shape function 2nd derivative to tr
	
	double* Grs(int n) { return ((FESolidElementTraits*)(m_pT))->Grs[n]; }	// shape function 2nd derivative to rs
	double* Gss(int n) { return ((FESolidElementTraits*)(m_pT))->Gss[n]; }	// shape function 2nd derivative to ss
	double* Gts(int n) { return ((FESolidElementTraits*)(m_pT))->Gts[n]; }	// shape function 2nd derivative to ts
	
	double* Grt(int n) { return ((FESolidElementTraits*)(m_pT))->Grt[n]; }	// shape function 2nd derivative to rt
	double* Gst(int n) { return ((FESolidElementTraits*)(m_pT))->Gst[n]; }	// shape function 2nd derivative to st
	double* Gtt(int n) { return ((FESolidElementTraits*)(m_pT))->Gtt[n]; }	// shape function 2nd derivative to tt

	//! intialize element data
	void Init(bool bflag)
	{
		int nint = GaussPoints();
		for (int i=0; i<nint; ++i) m_State[i]->Init(bflag);
	}

	//! values of shape functions
	void shape_fnc(double* H, double r, double s, double t) { ((FESolidElementTraits*)(m_pT))->shape_fnc(H, r, s, t); }

	//! values of shape function derivatives
	void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t) { ((FESolidElementTraits*)(m_pT))->shape_deriv(Hr, Hs, Ht, r, s, t); }

	//! values of shape function second derivatives
	void shape_deriv2(double* Hrr, double* Hss, double* Htt, double* Hrs, double* Hst, double* Hrt, double r, double s, double t) { ((FESolidElementTraits*)(m_pT))->shape_deriv2(Hrr, Hss, Htt, Hrs, Hst, Hrt, r, s, t); }

	//! this function projects data from the gauss-points to the nodal points
	void project_to_nodes(double* ai, double* ao) { ((FESolidElementTraits*)m_pT)->project_to_nodes(ai, ao); }
};

//-----------------------------------------------------------------------------
//!  This class defines a surface element

class FESurfaceElement : public FEElement
{
public:
	FESurfaceElement();

	FESurfaceElement(const FESurfaceElement& el);

	FESurfaceElement& operator = (const FESurfaceElement& el);

	virtual void SetTraits(FEElementTraits* pt);

	double* GaussWeights() { return &((FESurfaceElementTraits*)(m_pT))->gw[0]; }			// weights of integration points
	double gr(int n) { return ((FESurfaceElementTraits*)(m_pT))->gr[n]; }	// integration point coordinate r
	double gs(int n) { return ((FESurfaceElementTraits*)(m_pT))->gs[n]; }	// integration point coordinate  s

	double* Gr(int n) { return ((FESurfaceElementTraits*)(m_pT))->Gr[n]; }	// shape function derivative to r
	double* Gs(int n) { return ((FESurfaceElementTraits*)(m_pT))->Gs[n]; }	// shape function derivative to s

	double eval(double* d, int n)
	{
		double* N = H(n);
		int ne = Nodes();
		double a = 0;
		for (int i=0; i<ne; ++i) a += N[i]*d[i];
		return a;
	}

	double eval(double* d, double r, double s)
	{
		int n = Nodes();
		double H[FEElement::MAX_NODES];
		shape_fnc(H, r, s);
		double a = 0;
		for (int i=0; i<n; ++i) a += H[i]*d[i];
		return a;
	}

	vec3d eval(vec3d* d, double r, double s)
	{
		int n = Nodes();
		double H[FEElement::MAX_NODES];
		shape_fnc(H, r, s);
		vec3d a(0,0,0);
		for (int i=0; i<n; ++i) a += d[i]*H[i];
		return a;
	}

	vec3d eval(vec3d* d, int n)
	{
		int ne = Nodes();
		double* N = H(n);
		vec3d a(0,0,0);
		for (int i=0; i<ne; ++i) a += d[i]*N[i];
		return a;
	}

	double eval_deriv1(double* d, int j)
	{
		double* Hr = Gr(j);
		int n = Nodes();
		double s = 0;
		for (int i=0; i<n; ++i) s +=  Hr[i]*d[i];
		return s;
	}

	double eval_deriv2(double* d, int j)
	{
		double* Hs = Gs(j);
		int n = Nodes();
		double s = 0;
		for (int i=0; i<n; ++i) s +=  Hs[i]*d[i];
		return s;
	}

	double eval_deriv1(double* d, double r, double s)
	{
		double Hr[FEElement::MAX_NODES], Hs[FEElement::MAX_NODES];
		shape_deriv(Hr, Hs, r, s);
		int n = Nodes();
		double a = 0;
		for (int i=0; i<n; ++i) a +=  Hr[i]*d[i];
		return a;
	}

	double eval_deriv2(double* d, double r, double s)
	{
		double Hr[FEElement::MAX_NODES], Hs[FEElement::MAX_NODES];
		shape_deriv(Hr, Hs, r, s);
		int n = Nodes();
		double a = 0;
		for (int i=0; i<n; ++i) a +=  Hs[i]*d[i];
		return a;
	}

	void shape_fnc(double* H, double r, double s)
	{
		((FESurfaceElementTraits*)m_pT)->shape(H, r, s);
	}

	void shape_deriv(double* Gr, double* Gs, double r, double s)
	{
		((FESurfaceElementTraits*)m_pT)->shape_deriv(Gr, Gs, r, s);
	}

	void shape_deriv2(double* Grr, double* Grs, double* Gss, double r, double s)
	{
		((FESurfaceElementTraits*)m_pT)->shape_deriv2(Grr, Grs, Gss, r, s);
	}

	//! this function projects data from the gauss-points to the nodal points
	void project_to_nodes(double* ai, double* ao)
	{
		((FESurfaceElementTraits*)m_pT)->project_to_nodes(ai, ao);
	}

	bool HasNode(int n)
	{ 
		int l = Nodes(); 
		for (int i=0; i<l; ++i) 
			if (m_node[i] == n) return true; 
		return false;
	}

public:
	//! local ID of surface element
	int		m_lid;

	// indices of solid or shell element this surface is a face of
	// For solids, a surface element can be connected to two elements 
	// if the surface is an inside surface. For boundary surfaces
	// the second element index is -1. 
	int		m_elem[2];				
};

//-----------------------------------------------------------------------------
//!  This class defines the shell element. 

//! A shell element is similar to a surface
//! element except that it has a thickness. 

class FEShellElement : public FEElement
{
public:
	FEShellElement(){}

	//! copy constructor
	FEShellElement(const FEShellElement& el);

	//! assignment operator
	FEShellElement& operator = (const FEShellElement& el);

	virtual void SetTraits(FEElementTraits* ptraits);

	double* GaussWeights() { return &((FEShellElementTraits*)(m_pT))->gw[0]; }	// weights of integration points

	double* Hr(int n) { return ((FEShellElementTraits*)(m_pT))->Hr[n]; }	// shape function derivative to r
	double* Hs(int n) { return ((FEShellElementTraits*)(m_pT))->Hs[n]; }	// shape function derivative to s

	void Init(bool bflag)
	{
		int nint = GaussPoints();
		for (int i=0; i<nint; ++i) m_State[i]->Init(bflag);
	}

	double gr(int n) { return ((FEShellElementTraits*)(m_pT))->gr[n]; }
	double gs(int n) { return ((FEShellElementTraits*)(m_pT))->gs[n]; }
	double gt(int n) { return ((FEShellElementTraits*)(m_pT))->gt[n]; }

public:
	vector<double>	m_h0;	//!< initial shell thicknesses
	vector<vec3d>	m_D0;	//!< initial shell directors
};

//-----------------------------------------------------------------------------

class FETrussElement : public FEElement
{
public:
	FETrussElement();

	FETrussElement(const FETrussElement& el);

	FETrussElement& operator = (const FETrussElement& el);

	//! intialize element data
	void Init(bool bflag)
	{
		m_State[0]->Init(bflag);
	}

public:
	double	m_a0;	// cross-sectional area
};

//-----------------------------------------------------------------------------
//! Discrete element class

class FEDiscreteElement : public FEElement
{
public:
	FEDiscreteElement(){}
	FEDiscreteElement(const FEDiscreteElement& e);
	FEDiscreteElement& operator = (const FEDiscreteElement& e);
};

//-----------------------------------------------------------------------------
//!  This class defines a 2D element
class FEElement2D : public FEElement
{
public:
	//! default constructor
	FEElement2D(){}

	//! copy constructor
	FEElement2D(const FEElement2D& el);

	//! assignment operator
	FEElement2D& operator = (const FEElement2D& el);

	double* GaussWeights() { return &((FE2DElementTraits*)(m_pT))->gw[0]; }			// weights of integration points

	double* Hr(int n) { return ((FE2DElementTraits*)(m_pT))->Gr[n]; }	// shape function derivative to r
	double* Hs(int n) { return ((FE2DElementTraits*)(m_pT))->Gs[n]; }	// shape function derivative to s

    double* Hrr(int n) { return ((FE2DElementTraits*)(m_pT))->Grr[n]; }	// shape function 2nd derivative to rr
    double* Hsr(int n) { return ((FE2DElementTraits*)(m_pT))->Gsr[n]; }	// shape function 2nd derivative to sr
    
    double* Hrs(int n) { return ((FE2DElementTraits*)(m_pT))->Grs[n]; }	// shape function 2nd derivative to rs
    double* Hss(int n) { return ((FE2DElementTraits*)(m_pT))->Gss[n]; }	// shape function 2nd derivative to ss
    
	//! intialize element data
	void Init(bool bflag)
	{
		int nint = GaussPoints();
		for (int i=0; i<nint; ++i) m_State[i]->Init(bflag);
	}

	//! values of shape functions
	void shape_fnc(double* H, double r, double s) { ((FE2DElementTraits*)(m_pT))->shape(H, r, s); }

	//! values of shape function derivatives
	void shape_deriv(double* Hr, double* Hs, double r, double s) { ((FE2DElementTraits*)(m_pT))->shape_deriv(Hr, Hs, r, s); }

	//! this function projects data from the gauss-points to the nodal points
	void project_to_nodes(double* ai, double* ao) { ((FE2DElementTraits*)m_pT)->project_to_nodes(ai, ao); }
};

//-----------------------------------------------------------------------------
class FELineElement : public FEElement
{
public:
	FELineElement();

	FELineElement(const FELineElement& el);

	FELineElement& operator = (const FELineElement& el);

	void SetTraits(FEElementTraits* pt);

public:
	int	m_lid;	//!< local ID
};

#endif // !defined(AFX_FEELEMENT_H__2EE38101_58E2_4FEB_B214_BB71B6FB15FB__INCLUDED_)
