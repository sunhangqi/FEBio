// FEMesh.cpp: implementation of the FEMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMesh.h"
#include "FEException.h"
#include "FEDiscreteDomain.h"
#include "FETrussDomain.h"
#include "FEShellDomain.h"
#include "FESolidDomain.h"
#include "FEMaterial.h"
#include "log.h"
#include "DOFS.h"

//=============================================================================
// FENode
//-----------------------------------------------------------------------------
FENode::FENode()
{
	// initialize nodal data
	m_p0 = 0;
	m_pt = 0;
	m_T = 0;
	m_T0 = 0;
    
    // get DOFS
    DOFS& fedofs = *DOFS::GetInstance();
    int MAX_NDOFS = fedofs.GetNDOFS();
    int MAX_CDOFS = fedofs.GetCDOFS();
    
    m_c0.assign(MAX_CDOFS, 0);
    m_ct.assign(MAX_CDOFS, 0);
    m_cp.assign(MAX_CDOFS, 0);

	// initialize dof stuff
    m_ID.assign(MAX_NDOFS, DOF_FIXED);

	// exclude flag (true if the node should not be part of the analysis.
	// For instance, if it is isolated).
	m_bexclude = false;

	// shell flag
	m_bshell = false;

	// rigid body data
	m_rid = -1;
}

//-----------------------------------------------------------------------------
FENode::FENode(const FENode& n)
{
	m_r0 = n.m_r0;
	m_v0 = n.m_v0;
	m_rt = n.m_rt;
	m_vt = n.m_vt;
	m_at = n.m_at;
	m_rp = n.m_rp;
	m_vp = n.m_vp;
	m_ap = n.m_ap;
	m_Fr = n.m_Fr;
	m_D0 = n.m_D0;
	m_Dt = n.m_Dt;
	m_p0 = n.m_p0;
	m_pt = n.m_pt;
	m_T  = n.m_T;
	m_T0 = n.m_T0;
	m_c0 = n.m_c0;
	m_ct = n.m_ct;
	m_cp = n.m_cp;

	m_rid = n.m_rid;
	m_bshell = n.m_bshell;
	m_bexclude = n.m_bexclude;

	m_ID = n.m_ID;
}

//-----------------------------------------------------------------------------
FENode& FENode::operator = (const FENode& n)
{
	m_r0 = n.m_r0;
	m_v0 = n.m_v0;
	m_rt = n.m_rt;
	m_vt = n.m_vt;
	m_at = n.m_at;
	m_rp = n.m_rp;
	m_vp = n.m_vp;
	m_ap = n.m_ap;
	m_Fr = n.m_Fr;
	m_D0 = n.m_D0;
	m_Dt = n.m_Dt;
	m_p0 = n.m_p0;
	m_pt = n.m_pt;
	m_T  = n.m_T;
	m_T0 = n.m_T0;
	m_c0 = n.m_c0;
	m_ct = n.m_ct;
	m_cp = n.m_cp;

	m_rid = n.m_rid;
	m_bshell = n.m_bshell;
	m_bexclude = n.m_bexclude;

	m_ID = n.m_ID;

	return (*this);
}

//=============================================================================
// FENodeSet
//-----------------------------------------------------------------------------
FENodeSet::FENodeSet(FEMesh* pm) : m_pmesh(pm), m_nID(-1)
{ 
	m_szname[0] = 0; 
}

//-----------------------------------------------------------------------------
void FENodeSet::create(int n)
{
	assert(n);
	m_Node.resize(n);
}

//-----------------------------------------------------------------------------
void FENodeSet::SetName(const char* sz)
{
	strcpy(m_szname, sz); 
}

//=============================================================================
// FEFacetSet
//-----------------------------------------------------------------------------
FEFacetSet::FEFacetSet()
{
	m_szname[0] = 0;
}

//-----------------------------------------------------------------------------
void FEFacetSet::Create(int n)
{
	m_Face.resize(n);
}

//-----------------------------------------------------------------------------
FEFacetSet::FACET& FEFacetSet::Face(int i)
{
	return m_Face[i];
}

//-----------------------------------------------------------------------------
void FEFacetSet::SetName(const char* sz)
{
	strcpy(m_szname, sz); 
}

//=============================================================================
// FEElementSet
//-----------------------------------------------------------------------------
FEElementSet::FEElementSet(FEMesh* pm) : m_pmesh(pm)
{
	m_szname[0] = 0;
}

//-----------------------------------------------------------------------------
void FEElementSet::create(int n)
{
	assert(n);
	m_Elem.resize(n);
}

//-----------------------------------------------------------------------------
void FEElementSet::SetName(const char* sz)
{
	strcpy(m_szname, sz); 
}

//=============================================================================
// FEMesh
//-----------------------------------------------------------------------------
FEMesh::FEMesh()
{
}

//-----------------------------------------------------------------------------
FEMesh::~FEMesh()
{
	for (size_t i=0; i<m_NodeSet.size(); ++i) delete m_NodeSet[i];
	for (size_t i=0; i<m_FaceSet.size(); ++i) delete m_FaceSet[i];
	for (size_t i=0; i<m_ElSet.size()  ; ++i) delete m_ElSet[i];
	m_NodeSet.clear();
	m_FaceSet.clear();
	m_ElSet.clear();

	ClearDomains();
}

//-----------------------------------------------------------------------------
void FEMesh::ClearDomains()
{
	// clear solid domains
	for (size_t i=0; i<m_Domain.size(); ++i) delete m_Domain[i];
	m_Domain.clear();
}

//-----------------------------------------------------------------------------
void FEMesh::ShallowCopy(DumpStream& dmp, bool bsave)
{
 	// stream nodal data
	if (bsave)
	{
		int NN = (int) m_Node.size();
		for (int i=0; i<NN; ++i)
		{
			FENode& nd = m_Node[i];
			dmp << nd.m_r0 << nd.m_v0;
			dmp << nd.m_rt << nd.m_vt << nd.m_at;
			dmp << nd.m_rp << nd.m_vp << nd.m_ap;
			dmp << nd.m_Fr;
			dmp << nd.m_D0 << nd.m_Dt;
			dmp << nd.m_p0 << nd.m_pt;
			dmp << nd.m_T;
			dmp << nd.m_c0;
			dmp << nd.m_ct;
			dmp << nd.m_cp;
		}
	}
	else
	{
		int NN = (int) m_Node.size();
		for (int i=0; i<NN; ++i)
		{
			FENode& nd = m_Node[i];
			dmp >> nd.m_r0 >> nd.m_v0;
			dmp >> nd.m_rt >> nd.m_vt >> nd.m_at;
			dmp >> nd.m_rp >> nd.m_vp >> nd.m_ap;
			dmp >> nd.m_Fr;
			dmp >> nd.m_D0 >> nd.m_Dt;
			dmp >> nd.m_p0 >> nd.m_pt;
			dmp >> nd.m_T;
			dmp >> nd.m_c0;
			dmp >> nd.m_ct;
			dmp >> nd.m_cp;
		}
	}

	// stream domain data
	int ND = Domains();
	for (int i=0; i<ND; ++i)
	{
		FEDomain& dom = Domain(i);
		dom.ShallowCopy(dmp, bsave);
	}
}

//-----------------------------------------------------------------------------
//  Allocates storage for mesh data.
//
void FEMesh::CreateNodes(int nodes)
{
	assert(nodes);
	m_Node.resize (nodes);
}

//-----------------------------------------------------------------------------
// Make more room for nodes
void FEMesh::AddNodes(int nodes)
{
	assert(nodes);
	int N0 = (int) m_Node.size();
	m_Node.resize(N0 + nodes);
}

//-----------------------------------------------------------------------------
//! Return the total number elements
int FEMesh::Elements()
{
	int N = 0;
	for (int i=0; i<(int) m_Domain.size(); ++i) 
	{
		N += m_Domain[i]->Elements();
	}
	return N;
}

//-----------------------------------------------------------------------------
//! Return the number of solid elements
int FEMesh::SolidElements()
{
	int N = 0;
	for (int i=0; i<(int) m_Domain.size(); ++i)
	{
		if (m_Domain[i]->Class() == FE_DOMAIN_SOLID)
		{
			FESolidDomain& sd = static_cast<FESolidDomain&>(*m_Domain[i]);
			N += sd.Elements();
		}
	}
	return N;
}

//-----------------------------------------------------------------------------
//! Return the number of shell elements
int FEMesh::ShellElements()
{
	int N = 0;
	for (int i=0; i<(int) m_Domain.size(); ++i)
	{
		if (m_Domain[i]->Class() == FE_DOMAIN_SHELL)
		{
			FEShellDomain& sd = static_cast<FEShellDomain&>(*m_Domain[i]);
			N += sd.Elements();
		}
	}
	return N;
}

//-----------------------------------------------------------------------------
//! Return the number of truss elements
int FEMesh::TrussElements()
{
	int N = 0;
	for (int i=0; i<(int) m_Domain.size(); ++i)
	{
		if (m_Domain[i]->Class() == FE_DOMAIN_TRUSS)
		{
			FETrussDomain& td = static_cast<FETrussDomain&>(*m_Domain[i]);
			N += td.Elements();
		}
	}
	return N;
}

//-----------------------------------------------------------------------------
//! Return the number of discrete elements
int FEMesh::DiscreteElements()
{
	int N = 0;
	for (int i=0; i<(int) m_Domain.size(); ++i)
	{
		if (m_Domain[i]->Class() == FE_DOMAIN_DISCRETE)
		{
			FEDiscreteDomain& dd = static_cast<FEDiscreteDomain&>(*m_Domain[i]);
			N += dd.Elements();
		}
	}
	return N;
}

//-----------------------------------------------------------------------------
//  Updates the bounding box of the mesh (using current coordinates)
//
void FEMesh::UpdateBox()
{
	vec3d r0, r1;

	r0 = r1 = Node(0).m_rt;
	for (int i=1; i<Nodes(); ++i)
	{
		vec3d r = Node(i).m_rt;

		if (r.x < r0.x) r0.x = r.x;
		if (r.y < r0.y) r0.y = r.y;
		if (r.z < r0.z) r0.z = r.z;

		if (r.x > r1.x) r1.x = r.x;
		if (r.y > r1.y) r1.y = r.y;
		if (r.z > r1.z) r1.z = r.z;
	}

	m_box.r0 = r0;
	m_box.r1 = r1;
}

//-----------------------------------------------------------------------------
//  Counts the number of shell elements in the mesh
//
int FEMesh::RemoveIsolatedVertices()
{
	int i, j, k, N = Nodes(), n;

	// create a valence array
	vector<int> val; val.assign(N, 0);

	// count the nodal valences
	for (i=0; i<(int) m_Domain.size(); ++i)
	{
		FEDomain& d = Domain(i);
		for (j=0; j<d.Elements(); ++j)
		{
			FEElement& el = d.ElementRef(j);
			n = el.Nodes();
			for (k=0; k<n; ++k) ++val[el.m_node[k]];
		}
	}

	// See if there are any isolated nodes
	// Exclude them from the analysis
	int ni = 0;
	for (i=0; i<N; ++i)
		if (val[i] == 0)
		{
			++ni;
			FENode& node = Node(i);
			node.m_bexclude = true;
		}

	return ni;
}

//-----------------------------------------------------------------------------
//! Calculate all shell normals (i.e. the shell directors).
void FEMesh::InitShellNormals()
{
	// zero initial directors for shell nodes
	for (int i=0; i<Nodes(); ++i) Node(i).m_D0 = vec3d(0,0,0);

	// loop over all domains
	for (int nd = 0; nd < Domains(); ++nd)
	{
		// Calculate the shell directors as the local node normals
		if (Domain(nd).Class() == FE_DOMAIN_SHELL)
		{
			FEShellDomain& sd = static_cast<FEShellDomain&>(Domain(nd));
			vec3d r0[FEElement::MAX_NODES];
			for (int i=0; i<sd.Elements(); ++i)
			{
				FEShellElement& el = sd.Element(i);

				int n = el.Nodes();
				int* en = &el.m_node[0];

				// get the nodes
				for (int j=0; j<n; ++j) r0[j] = Node(en[j]).m_r0;

				for (int j=0; j<n; ++j)
				{
					int m0 = j;
					int m1 = (j+1)%n;
					int m2 = (j==0? n-1: j-1);

					vec3d a = r0[m0];
					vec3d b = r0[m1];
					vec3d c = r0[m2];

					Node(en[m0]).m_D0 += (b-a)^(c-a);
				}
			}
		}
	}

	// make sure we start with unit directors
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		node.m_D0.unit();
		node.m_Dt = node.m_D0;
	}
}


//-----------------------------------------------------------------------------
//! Does one-time initialization of the Mesh data. Call FEMesh::Reset for resetting 
//! the mesh data.
bool FEMesh::Init()
{
	// find and remove isolated vertices
	int ni = RemoveIsolatedVertices();
	if (ni != 0) 
	{
		if (ni == 1)
			felog.printbox("WARNING", "%d isolated vertex removed.", ni);
		else
			felog.printbox("WARNING", "%d isolated vertices removed.", ni);
	}

	// Initialize shell normals (i.e. directors)
	// NOTE: we do this before we check for inverted elements since the jacobian of a shell
	//       depends on its normal.
	InitShellNormals();

	// reset data
	// TODO: Not sure why this is here
	Reset();

	// All done
	return true;
}

//-----------------------------------------------------------------------------
//! Reset the mesh data. Return nodes to their intial position, reset their 
//! attributes and zero all element stresses.

void FEMesh::Reset()
{
	// reset nodal data
	for (int i=0; i<Nodes(); ++i) 
	{
		FENode& node = Node(i);

		node.m_rp = node.m_rt = node.m_r0;
		node.m_vp = node.m_vt = node.m_v0;
		node.m_ap = node.m_at = vec3d(0,0,0);

		node.m_pt = node.m_p0;
		
		int cdofs = (int) node.m_ct.size();
		for (int k=0; k<cdofs; ++k)
			node.m_ct[k] = node.m_cp[k] = node.m_c0[k];
		
		node.m_T = node.m_T0;

		node.m_Fr = vec3d(0,0,0);
		node.m_Dt = node.m_D0;

		// reset ID arrays
		int ndof = node.m_ID.size();
		for (int i=0; i<ndof; ++i) node.m_ID[i] = DOF_FIXED;
	}

	// update the mesh
	UpdateBox();

	// reset domain data
	for (int n=0; n<(int) m_Domain.size(); ++n) m_Domain[n]->Reset();
}

//-----------------------------------------------------------------------------
//! This function calculates the (initial) volume of an element. In some case, the volume
//! may only be approximate.
double FEMesh::ElementVolume(FEElement &el)
{
	double V = 0;
	switch (el.Class())
	{
	case FE_ELEM_SOLID: V = SolidElementVolume(static_cast<FESolidElement&>(el)); break;
	case FE_ELEM_SHELL: V = ShellElementVolume(static_cast<FEShellElement&>(el)); break;
	}
	return V;
}

//-----------------------------------------------------------------------------
//! \todo Replace this with what FEBio 1.x does.
double FEMesh::SolidElementVolume(FESolidElement& el)
{
	int i;
	vec3d r0[FEElement::MAX_NODES];

	int neln = el.Nodes();
	for (i=0; i<neln; ++i) r0[i] = Node(el.m_node[i]).m_r0;

	int nint = el.GaussPoints();
	double *w = el.GaussWeights();
	double V = 0;
	for (int n=0; n<nint; ++n) 
	{
		// shape function derivatives
		double* Grn = el.Gr(n);
		double* Gsn = el.Gs(n);
		double* Gtn = el.Gt(n);

		// jacobian matrix
		double J[3][3] = {0};
		for (i=0; i<neln; ++i)
		{
			const double& Gri = Grn[i];
			const double& Gsi = Gsn[i];
			const double& Gti = Gtn[i];
			
			const double& x = r0[i].x;
			const double& y = r0[i].y;
			const double& z = r0[i].z;
			
			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}
			
		// calculate the determinant
		double detJ0 =  J[0][0]*(J[1][1]*J[2][2] - J[1][2]*J[2][1]) 
					+ J[0][1]*(J[1][2]*J[2][0] - J[2][2]*J[1][0]) 
					+ J[0][2]*(J[1][0]*J[2][1] - J[1][1]*J[2][0]);

		V += detJ0*w[n];
	}

	return V;
}

//-----------------------------------------------------------------------------
//! \todo Replace this with what FEBio 1.x does.
double FEMesh::ShellElementVolume(FEShellElement& el)
{
	int i;
	int neln = el.Nodes();

	// initial nodal coordinates and directors
	vec3d r0[FEElement::MAX_NODES], D0[FEElement::MAX_NODES];
	for (i=0; i<neln; ++i)
	{
		r0[i] = Node(el.m_node[i]).m_r0;
		D0[i] = Node(el.m_node[i]).m_D0;
	}

	int nint = el.GaussPoints();
	double *w = el.GaussWeights();
	double V = 0;
	for (int n=0; n<nint; ++n)
	{
		// jacobian matrix
		double* h0 = &el.m_h0[0];
		double gt = el.gt(n);
		double J[3][3] = {0};
		for (i=0; i<neln; ++i)
		{
			const double& Hri = el.Hr(n)[i];
			const double& Hsi = el.Hs(n)[i];
			const double& Hi = el.H(n)[i];
			
			const double& x = r0[i].x;
			const double& y = r0[i].y;
			const double& z = r0[i].z;
			
			const double& dx = D0[i].x;
			const double& dy = D0[i].y;
			const double& dz = D0[i].z;
			
			double za = 0.5*gt*h0[i];
			
			J[0][0] += Hri*x + Hri*za*dx; J[0][1] += Hsi*x + Hsi*za*dx; J[0][2] += 0.5*h0[i]*Hi*dx;
			J[1][0] += Hri*y + Hri*za*dy; J[1][1] += Hsi*y + Hsi*za*dy; J[1][2] += 0.5*h0[i]*Hi*dy;
			J[2][0] += Hri*z + Hri*za*dz; J[2][1] += Hsi*z + Hsi*za*dz; J[2][2] += 0.5*h0[i]*Hi*dz;
		}
				
		// calculate the determinant
		double detJ0 =  J[0][0]*(J[1][1]*J[2][2] - J[1][2]*J[2][1]) 
					  + J[0][1]*(J[1][2]*J[2][0] - J[2][2]*J[1][0]) 
					  + J[0][2]*(J[1][0]*J[2][1] - J[1][1]*J[2][0]);

		V += detJ0*w[n];
	}

	return V;
}

//-----------------------------------------------------------------------------
//! Find a nodeset by ID

FENodeSet* FEMesh::FindNodeSet(int nid)
{
	for (size_t i=0; i<m_NodeSet.size(); ++i) if (m_NodeSet[i]->GetID() == nid) return m_NodeSet[i];
	return 0;
}

//-----------------------------------------------------------------------------
//! Find a nodeset by name

FENodeSet* FEMesh::FindNodeSet(const char* szname)
{
	for (size_t i=0; i<m_NodeSet.size(); ++i) if (strcmp(m_NodeSet[i]->GetName(), szname) == 0) return m_NodeSet[i];
	return 0;
}

//-----------------------------------------------------------------------------
//! Find a element set by name

FEElementSet* FEMesh::FindElementSet(const char* szname)
{
	for (size_t i=0; i<m_ElSet.size(); ++i) if (strcmp(m_ElSet[i]->GetName(), szname) == 0) return m_ElSet[i];
	return 0;
}

//-----------------------------------------------------------------------------
int FEMesh::Faces(FEElement& el)
{
	switch (el.Type())
	{
	case FE_HEX8G8:
	case FE_HEX8RI:
	case FE_HEX8G1:
	case FE_HEX20G27: 
	case FE_HEX27G27: return 6;
	case FE_PENTA6G6: return 5;
	case FE_TET4G4:
	case FE_TET10G4:
	case FE_TET10G8:
	case FE_TET10GL11:
	case FE_TET15G8:
	case FE_TET15G11:
	case FE_TET15G15:
	case FE_TET4G1: return 4;
	case FE_SHELL_QUAD:
	case FE_SHELL_TRI: return 1;
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
//! This function returns the face connectivity from a certain element

int FEMesh::GetFace(FEElement& el, int n, int* nf)
{
	int nn = -1;
	int* en = &el.m_node[0];
	switch (el.Type())
	{
	case FE_HEX8G8:
	case FE_HEX8RI:
	case FE_HEX8G1:
		nn = 4;
		switch (n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[5]; nf[3] = en[4]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[6]; nf[3] = en[5]; break;
		case 2: nf[0] = en[2]; nf[1] = en[3]; nf[2] = en[7]; nf[3] = en[6]; break;
		case 3: nf[0] = en[0]; nf[1] = en[4]; nf[2] = en[7]; nf[3] = en[3]; break;
		case 4: nf[0] = en[0]; nf[1] = en[3]; nf[2] = en[2]; nf[3] = en[1]; break;
		case 5: nf[0] = en[4]; nf[1] = en[5]; nf[2] = en[6]; nf[3] = en[7]; break;
		}
		break;
	case FE_PENTA6G6:
		switch(n)
		{
		case 0: nn = 4; nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[4]; nf[3] = en[3]; break;
		case 1: nn = 4; nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[5]; nf[3] = en[4]; break;
		case 2: nn = 4; nf[0] = en[0]; nf[1] = en[3]; nf[2] = en[5]; nf[3] = en[2]; break;
		case 3: nn = 3; nf[0] = en[0]; nf[1] = en[2]; nf[2] = en[1]; nf[3] = en[1]; break;
		case 4: nn = 3; nf[0] = en[3]; nf[1] = en[4]; nf[2] = en[5]; nf[3] = en[5]; break;
		}
		break;
	case FE_TET4G4:
	case FE_TET4G1:
		nn = 3;
		switch (n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = nf[3] = en[3]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = nf[3] = en[3]; break;
		case 2: nf[0] = en[0]; nf[1] = en[3]; nf[2] = nf[3] = en[2]; break;
		case 3: nf[0] = en[0]; nf[1] = en[2]; nf[2] = nf[3] = en[1]; break;
		}
		break;
	case FE_TET10G4:
	case FE_TET10G8:
	case FE_TET10GL11:
		nn = 6;
		switch(n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[3]; nf[3] = en[4]; nf[4] = en[8]; nf[5] = en[7]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[3]; nf[3] = en[5]; nf[4] = en[9]; nf[5] = en[8]; break;
		case 2: nf[0] = en[2]; nf[1] = en[0]; nf[2] = en[3]; nf[3] = en[6]; nf[4] = en[7]; nf[5] = en[9]; break;
		case 3: nf[0] = en[2]; nf[1] = en[1]; nf[2] = en[0]; nf[3] = en[5]; nf[4] = en[4]; nf[5] = en[6]; break;
		}
		break;
	case FE_TET15G8:
	case FE_TET15G11:
	case FE_TET15G15:
		nn = 7;
		switch(n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[3]; nf[3] = en[4]; nf[4] = en[8]; nf[5] = en[7]; nf[6] = en[11]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[3]; nf[3] = en[5]; nf[4] = en[9]; nf[5] = en[8]; nf[6] = en[12]; break;
		case 2: nf[0] = en[2]; nf[1] = en[0]; nf[2] = en[3]; nf[3] = en[6]; nf[4] = en[7]; nf[5] = en[9]; nf[6] = en[13]; break;
		case 3: nf[0] = en[2]; nf[1] = en[1]; nf[2] = en[0]; nf[3] = en[5]; nf[4] = en[4]; nf[5] = en[6]; nf[6] = en[10]; break;
		}
		break;
	case FE_HEX20G27:
		nn = 8;
		switch(n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[5]; nf[3] = en[4]; nf[4] = en[ 8]; nf[5] = en[17]; nf[6] = en[12]; nf[7] = en[16]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[6]; nf[3] = en[5]; nf[4] = en[ 9]; nf[5] = en[18]; nf[6] = en[13]; nf[7] = en[17]; break;
		case 2: nf[0] = en[2]; nf[1] = en[3]; nf[2] = en[7]; nf[3] = en[6]; nf[4] = en[10]; nf[5] = en[19]; nf[6] = en[14]; nf[7] = en[18]; break;
		case 3: nf[0] = en[0]; nf[1] = en[4]; nf[2] = en[7]; nf[3] = en[3]; nf[4] = en[16]; nf[5] = en[15]; nf[6] = en[19]; nf[7] = en[11]; break;
		case 4: nf[0] = en[0]; nf[1] = en[3]; nf[2] = en[2]; nf[3] = en[1]; nf[4] = en[11]; nf[5] = en[10]; nf[6] = en[ 9]; nf[7] = en[ 8]; break;
		case 5: nf[0] = en[4]; nf[1] = en[5]; nf[2] = en[6]; nf[3] = en[7]; nf[4] = en[12]; nf[5] = en[13]; nf[6] = en[14]; nf[7] = en[15]; break;
		}
		break;
	case FE_HEX27G27:
		nn = 9;
		switch(n)
		{
		case 0: nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[5]; nf[3] = en[4]; nf[4] = en[ 8]; nf[5] = en[17]; nf[6] = en[12]; nf[7] = en[16]; nf[8] = en[20]; break;
		case 1: nf[0] = en[1]; nf[1] = en[2]; nf[2] = en[6]; nf[3] = en[5]; nf[4] = en[ 9]; nf[5] = en[18]; nf[6] = en[13]; nf[7] = en[17]; nf[8] = en[21]; break;
		case 2: nf[0] = en[2]; nf[1] = en[3]; nf[2] = en[7]; nf[3] = en[6]; nf[4] = en[10]; nf[5] = en[19]; nf[6] = en[14]; nf[7] = en[18]; nf[8] = en[22]; break;
		case 3: nf[0] = en[0]; nf[1] = en[4]; nf[2] = en[7]; nf[3] = en[3]; nf[4] = en[16]; nf[5] = en[15]; nf[6] = en[19]; nf[7] = en[11]; nf[8] = en[23]; break;
		case 4: nf[0] = en[0]; nf[1] = en[3]; nf[2] = en[2]; nf[3] = en[1]; nf[4] = en[11]; nf[5] = en[10]; nf[6] = en[ 9]; nf[7] = en[ 8]; nf[8] = en[24]; break;
		case 5: nf[0] = en[4]; nf[1] = en[5]; nf[2] = en[6]; nf[3] = en[7]; nf[4] = en[12]; nf[5] = en[13]; nf[6] = en[14]; nf[7] = en[15]; nf[8] = en[25]; break;
		}
		break;
	case FE_SHELL_QUAD:
		nn = 4;
		nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[2]; nf[3] = en[3];
		break;
	case FE_SHELL_TRI:
		nn = 3;
		nf[0] = en[0]; nf[1] = en[1]; nf[2] = en[2];
		break;
	}

	return nn;
}

//-----------------------------------------------------------------------------
//! Find an element from a given ID. return 0 if the element cannot be found.

FEElement* FEMesh::FindElementFromID(int nid)
{
	FEElement* pe = 0;

	for (int i=0; i<Domains(); ++i)
	{
		FEDomain& d = Domain(i);
		pe = d.FindElementFromID(nid);
		if (pe) return pe;
	}

	return pe;
}

//-----------------------------------------------------------------------------
// Find the element in which point y lies
FESolidElement* FEMesh::FindSolidElement(vec3d y, double r[3])
{
	int ND = (int) m_Domain.size();
	for (int i=0; i<ND; ++i)
	{
		if (m_Domain[i]->Class() == FE_DOMAIN_SOLID)
		{
			FESolidDomain& bd = static_cast<FESolidDomain&>(*m_Domain[i]);
			FESolidElement* pe = bd.FindElement(y, r);
			if (pe) return pe;
		}
	}
	return 0;
}


//-----------------------------------------------------------------------------
//! This function finds all the domains that have a certain material
void FEMesh::DomainListFromMaterial(vector<int>& lmat, vector<int>& ldom)
{
	// make sure the list is empty
	if (ldom.empty() == false) ldom.clear();

	// loop over all domains
	int ND = (int) m_Domain.size();
	int NM = (int) lmat.size();
	for (int i=0; i<ND; ++i)
	{
		FEDomain& di = *m_Domain[i];
		int dmat = di.GetMaterial()->GetID();
		for (int j=0; j<NM; ++j)
		{
			if (dmat == lmat[j])
			{
				ldom.push_back(i);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! Copy the mesh data from the passed mesh object
void FEMesh::CopyFrom(FEMesh& mesh)
{
	// copy nodes
	m_Node = mesh.m_Node;

	// copy domains
	// NOTE: This creates domains that don't have materials assigned to them
	ClearDomains();
	int ND = mesh.Domains();
	for (int i=0; i<ND; ++i)
	{
		// copy this domain
		FEDomain* pd = mesh.Domain(i).Copy();
		assert(pd);

		// assign the domain to this mesh
		pd->SetMesh(this);

		// add the domain
		m_Domain.push_back(pd);
	}	
}
