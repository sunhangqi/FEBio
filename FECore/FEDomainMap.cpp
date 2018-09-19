#include "stdafx.h"
#include "FEDomainMap.h"

//-----------------------------------------------------------------------------
FEDomainMap::FEDomainMap(int dataType) : FEDataMap(dataType)
{
	m_maxElemNodes = 0;
}

//-----------------------------------------------------------------------------
FEDomainMap::FEDomainMap(const FEDomainMap& map) : FEDataMap(map), m_name(map.m_name)
{
	m_maxElemNodes = map.m_maxElemNodes;
}

//-----------------------------------------------------------------------------
FEDomainMap& FEDomainMap::operator = (const FEDomainMap& map)
{
	FEDataArray::operator=(map);
	m_name = map.m_name;
	m_maxElemNodes = map.m_maxElemNodes;
	return *this;
}

//-----------------------------------------------------------------------------
bool FEDomainMap::Create(const FEDomain* ps, double val)
{
	m_dom = ps;
	int NF = ps->Elements();
	m_maxElemNodes = 0;
	for (int i = 0; i<NF; ++i)
	{
		const FEElement& el = ps->ElementRef(i);
		int ne = el.Nodes();
		if (ne > m_maxElemNodes) m_maxElemNodes = ne;
	}
	return resize(NF*m_maxElemNodes, val);
}

//-----------------------------------------------------------------------------
void FEDomainMap::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
void FEDomainMap::setValue(int n, double v)
{
	int index = n*m_maxElemNodes;
	for (int i = 0; i<m_maxElemNodes; ++i) set<double>(index + i, v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::setValue(int n, const vec2d& v)
{
	int index = n*m_maxElemNodes;
	for (int i = 0; i<m_maxElemNodes; ++i) set<vec2d>(index + i, v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::setValue(int n, const vec3d& v)
{
	int index = n*m_maxElemNodes;
	for (int i = 0; i<m_maxElemNodes; ++i) set<vec3d>(index + i, v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::fillValue(double v)
{
	set<double>(v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::fillValue(const vec2d& v)
{
	set<vec2d>(v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::fillValue(const vec3d& v)
{
	set<vec3d>(v);
}

//-----------------------------------------------------------------------------
void FEDomainMap::Serialize(DumpStream& ar)
{
	FEDataArray::Serialize(ar);
	if (ar.IsSaving())
	{
		ar << m_maxElemNodes;
		ar << m_name;
	}
	else
	{
		ar >> m_maxElemNodes;
		ar >> m_name;
	}
}

//-----------------------------------------------------------------------------
//! get the value at a material point
double FEDomainMap::value(const FEMaterialPoint& pt)
{
	// get the element this material point is in
	FEElement* pe = pt.m_elem;
	assert(pe);

	// make sure this element belongs to this domain
	assert(pe->GetDomain() == m_dom);

	// get its local ID
	int lid = pe->GetLocalID();

	assert((lid >= 0) && (lid < (int)DataCount()));

	return get<double>(lid);
}
