#include "stdafx.h"
#include "FEParameterList.h"
#include "FECoreKernel.h"
#include "DumpStream.h"
#include <string>
#include <assert.h>

//-----------------------------------------------------------------------------
//! This function copies the parameter data from the passed parameter list.
//! This assumes that the two parameter lists are identical.
void FEParameterList::operator = (FEParameterList& l)
{
	if (m_pl.size() != l.m_pl.size()) { assert(false); return; }

	list<FEParam>::iterator ps, pd;
	ps = l.m_pl.begin();
	pd = m_pl.begin();
	for (; pd != m_pl.end(); ++pd, ++ps)
	{
		FEParam& s = *ps;
		FEParam& d = *pd;

		if (s.type() != d.type()) { assert(false); return; }
		if (s.dim() != d.dim()) { assert(false); return; }
		if (s.dim() == 1)
		{
			switch (s.type())
			{
			case FE_PARAM_INT   : d.value<int   >() = s.value<int   >(); break;
			case FE_PARAM_BOOL  : d.value<bool  >() = s.value<bool  >(); break;
			case FE_PARAM_DOUBLE: d.value<double>() = s.value<double>(); break;
			case FE_PARAM_VEC3D : d.value<vec3d >() = s.value<vec3d >(); break;
			case FE_PARAM_MAT3D : d.value<mat3d >() = s.value<mat3d >(); break;
			case FE_PARAM_MAT3DS: d.value<mat3ds>() = s.value<mat3ds>(); break;
			default:
				assert(false);
			}
		}
		else
		{
			switch (s.type())
			{
			case FE_PARAM_INT:
				{
					for (int i=0; i<s.dim(); ++i) d.pvalue<int>()[i] = s.pvalue<int>()[i];
				}
				break;
			case FE_PARAM_DOUBLE:
				{
					for (int i=0; i<s.dim(); ++i) d.pvalue<double>()[i] = s.pvalue<double>()[i];
				}
				break;
			default:
				assert(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// This function adds a parameter to the parameter list
void FEParameterList::AddParameter(void *pv, FEParamType itype, int ndim, const char *sz)
{
	// sanity checks
	assert(pv);
	assert(sz);

	// create a new parameter object
	FEParam p(pv, itype, ndim, sz);

	// add the parameter to the list
	m_pl.push_back(p);
}
//-----------------------------------------------------------------------------
// This function adds a parameter to the parameter list
void FEParameterList::AddParameter(void *pv, FEParamType itype, int ndim, FEParamRange rng, double fmin, double fmax, const char *sz)
{
	assert(pv);
	assert(sz);

	// create a new parameter object
	FEParam p(pv, itype, ndim, sz);

	// set the range
	// (range checking is only supported for int and double params)
	if (rng != FE_DONT_CARE)
	{
		if (itype == FE_PARAM_INT) p.SetValidator(new FEIntValidator(rng, (int) fmin, (int) fmax));
		else if (itype == FE_PARAM_DOUBLE) p.SetValidator(new FEDoubleValidator(rng, fmin, fmax));
	}

	// add the parameter to the list
	m_pl.push_back(p);
}

//-----------------------------------------------------------------------------
// Find a parameter using its data pointer
FEParam* FEParameterList::Find(void* pv)
{
	FEParam* pp = 0;
	if (m_pl.empty() == false)
	{
		list<FEParam>::iterator it;
		for (it = m_pl.begin(); it != m_pl.end(); ++it)
		{
			if (it->data_ptr() == pv)
			{
				pp = &(*it);
				break;
			}
		}
	}
	return pp;
}

//-----------------------------------------------------------------------------
// This function searches the parameters in the list for a parameter
// with the name given by the input argument
// \param sz name of parameter to find
FEParam* FEParameterList::Find(const char* sz)
{
	FEParam* pp = 0;
	if (m_pl.size() > 0)
	{
		list<FEParam>::iterator it;
		for (it = m_pl.begin(); it != m_pl.end(); ++it)
		{
			if (strcmp(it->name(), sz) == 0)
			{
				pp = &(*it);
				break;
			}
		}
	}

	return pp;
}

//=============================================================================

void cstrncpy(char* pd, char* ps, int l)
{
	for (int i=0; i<=l; ++i) pd[i] = ps[i];
}

ParamString::ParamString(const char* sz)
{
	m_sz = 0;
	m_nc = 0;
	m_nl = strlen(sz);
	m_sz = new char[m_nl+1];
	strcpy(m_sz, sz);
	char* ch = m_sz;
	while (ch = strchr(ch, '.')) { m_nc++; *ch = 0; ch = ch+1; }
	m_nc++;
}

//-----------------------------------------------------------------------------
ParamString::ParamString(const ParamString& p)
{
	m_nc = p.m_nc;
	m_nl = p.m_nl;
	m_sz = new char[m_nl+1];
	cstrncpy(m_sz, p.m_sz, m_nl);
}

//-----------------------------------------------------------------------------
void ParamString::operator=(const ParamString& p)
{
	m_nc = p.m_nc;
	m_nl = p.m_nl;
	delete [] m_sz;
	m_sz = new char[m_nl+1];
	cstrncpy(m_sz, p.m_sz, m_nl);
}

//-----------------------------------------------------------------------------
ParamString::~ParamString()
{
	delete [] m_sz;
	m_sz = 0;
	m_nc = 0;
	m_nl = 0;
}

//-----------------------------------------------------------------------------
int ParamString::count() const
{
	return m_nc;
}

//-----------------------------------------------------------------------------
ParamString ParamString::next() const
{
	int nc = m_nc - 1;
	int l = strlen(m_sz);
	char* sz = strchr(m_sz, '\0');
	char* sznew = new char[m_nl-l+1];
	cstrncpy(sznew, sz+1, m_nl-l);
	return ParamString(sznew, nc, m_nl-l);
}

//-----------------------------------------------------------------------------
bool ParamString::operator==(const char* sz) const
{
	return strcmp(m_sz, sz) == 0;
}

//=============================================================================
FEParamContainer::FEParamContainer()
{
	m_pParam = 0;
}

//-----------------------------------------------------------------------------
FEParamContainer::~FEParamContainer()
{
	delete m_pParam; 
	m_pParam = 0;
}

//-----------------------------------------------------------------------------
FEParameterList& FEParamContainer::GetParameterList()
{
	if (m_pParam == 0) 
	{
		m_pParam = new FEParameterList(this);
		BuildParamList();
	}
	return *m_pParam;
}

//-----------------------------------------------------------------------------
const FEParameterList& FEParamContainer::GetParameterList() const
{
	assert(m_pParam);
	return *m_pParam;
}

//-----------------------------------------------------------------------------
// Find a parameter from its name
FEParam* FEParamContainer::GetParameter(const ParamString& s)
{
	FEParameterList& pl = GetParameterList();
	return pl.Find(s.c_str());
}

//-----------------------------------------------------------------------------
FEParam* FEParamContainer::GetParameter(void* pv)
{
	FEParameterList& pl = GetParameterList();
	return pl.Find(pv);
}

//-----------------------------------------------------------------------------
// Add a parameter to the parameter list
void FEParamContainer::AddParameter(void* pv, FEParamType itype, int ndim, const char* sz)
{
	assert(m_pParam);
	m_pParam->AddParameter(pv, itype, ndim, sz);
}

//-----------------------------------------------------------------------------
// Add a parameter to the parameter list
void FEParamContainer::AddParameter(void* pv, FEParamType itype, int ndim, RANGE rng, const char* sz)
{
	assert(m_pParam);
	m_pParam->AddParameter(pv, itype, ndim, rng.m_rt, rng.m_fmin, rng.m_fmax, sz);
}

//-----------------------------------------------------------------------------
// Serialize parameters to archive
void FEParamContainer::Serialize(DumpStream& ar)
{
	if (ar.IsShallow()) return;

	if (ar.IsSaving())
	{
		int NP = 0;
		list<FEParam>::iterator it;
		if (m_pParam) // If the input file doesn't set any parameters, the parameter list won't be created.
		{
			NP = m_pParam->Parameters();
			it = m_pParam->first();
		}
		ar << NP;
		for (int i=0; i<NP; ++i)
		{
			FEParam& p = *it++;
			p.Serialize(ar);
		}
	}
	else
	{
		int NP = 0;
		ar >> NP;
		if (NP)
		{
			FEParameterList& pl = GetParameterList();
			if (NP != pl.Parameters()) throw DumpStream::ReadError();
			list<FEParam>::iterator it = pl.first();
			for (int i=0; i<NP; ++i)
			{
				FEParam& p = *it++;
				p.Serialize(ar);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! This function validates all parameters. 
//! It fails on the first parameter that is outside its allowed range.
//! Use fecore_get_error_string() to find out which parameter failed validation.
bool FEParamContainer::Validate()
{
	FEParameterList& pl = GetParameterList();
	int N = pl.Parameters();
	list<FEParam>::iterator pi = pl.first();
	for (int i=0; i<N; ++i, pi++)
	{
		FEParam& p = *pi;
		if (p.is_valid() == false)
		{
            string err = fecore_get_error_string();

			// report the error
            return fecore_error(err.c_str());
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEParamContainer::CopyParameterListState(const FEParameterList& pl)
{
	FEParameterList& pl_this = GetParameterList();
	assert(pl_this.Parameters() == pl.Parameters());
	int NP = pl.Parameters();
	FEParamIteratorConst it_s = pl.first();
	FEParamIterator it_d = pl_this.first();
	for (int i=0; i<NP; ++i, ++it_s, ++it_d)
	{
		const FEParam& ps = *it_s;
		FEParam& pd = *it_d;
		if (pd.CopyState(ps) == false) { assert(false); }
	}
}
