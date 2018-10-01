// FERigidBody.cpp: implementation of the FERigidBody class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FERigidBody.h"
#include <FECore/FEMaterial.h>
#include <FECore/FESolidDomain.h>
#include <FECore/FEModel.h>
#include "RigidBC.h"
#include "FERigidMaterial.h"

//-----------------------------------------------------------------------------
BEGIN_PARAMETER_LIST(FERigidBody, FEParamContainer);
	ADD_PARAMETER(m_Fr.x, "Fx");
	ADD_PARAMETER(m_Fr.y, "Fy");
	ADD_PARAMETER(m_Fr.z, "Fz");
	ADD_PARAMETER(m_Mr.x, "Mx");
	ADD_PARAMETER(m_Mr.y, "My");
	ADD_PARAMETER(m_Mr.z, "Mz");
	ADD_PARAMETER(m_euler, "euler");
	ADD_PARAMETER(m_r0, "initial_position");
	ADD_PARAMETER(m_rt, "position");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FERigidBody::FERigidBody(FEModel* pfem) : m_fem(*pfem)
{
    m_bpofr = false;
	for (int i=0; i<6; ++i)
	{
		m_pDC[i] = 0;
		m_LM[i] = -1;
		m_BC[i] = DOF_OPEN;
	}
	m_prb = 0;

	// zero total displacements
	m_Ut[0] = m_Up[0] = 0;
	m_Ut[1] = m_Up[1] = 0;
	m_Ut[2] = m_Up[2] = 0;
	m_Ut[3] = m_Up[3] = 0;
	m_Ut[4] = m_Up[4] = 0;
	m_Ut[5] = m_Up[5] = 0;

    // initialize velocity and acceleration of center of mass
    m_vt = m_at = vec3d(0,0,0);
    
	// initialize orientation
	m_qt = quatd(0, vec3d(0,0,1));
	m_euler = vec3d(0,0,0);
    
    // initialize angular velocity and acceleration
    m_wt = m_alt = vec3d(0,0,0);

	// initialize reaction forces
	m_Fr = m_Fp = vec3d(0,0,0);
	m_Mr = m_Mp = vec3d(0,0,0);
}

//-----------------------------------------------------------------------------
FERigidBody::~FERigidBody()
{

}

//-----------------------------------------------------------------------------
//! Reset rigid body data (called from FEM::Reset)
void FERigidBody::Reset()
{
	// zero total displacements
	m_Ut[0] = m_Up[0] = 0;
	m_Ut[1] = m_Up[1] = 0;
	m_Ut[2] = m_Up[2] = 0;
	m_Ut[3] = m_Up[3] = 0;
	m_Ut[4] = m_Up[4] = 0;
	m_Ut[5] = m_Up[5] = 0;

    // initialize velocity and acceleration of center of mass
    m_vp = m_vt = vec3d(0,0,0);
    m_ap = m_at = vec3d(0,0,0);
    
	// initialize orientation
	m_qp = m_qt = quatd(0, vec3d(0,0,1));
	m_euler = vec3d(0,0,0);

    // initialize angular velocity and acceleration
    m_wp = m_wt = vec3d(0,0,0);
    m_alp = m_alt = vec3d(0,0,0);
    
	// initialize center of mass
	m_rt = m_r0;

	// reset reaction force and torque
	m_Fr = vec3d(0,0,0);
	m_Mr = vec3d(0,0,0);
}

//-----------------------------------------------------------------------------
//! This function is called at the start of each time step and is used to update
//! some variables.
void FERigidBody::Init()
{
	// clear reaction forces
	m_Fr = m_Mr = vec3d(0,0,0);

	// store previous state
	m_rp = m_rt;
    m_vp = m_vt;
    m_ap = m_at;
	m_qp = m_qt;
    m_wp = m_wt;
    m_alp = m_alt;
	m_Up[0] = m_Ut[0];
	m_Up[1] = m_Ut[1];
	m_Up[2] = m_Ut[2];
	m_Up[3] = m_Ut[3];
	m_Up[4] = m_Ut[4];
	m_Up[5] = m_Ut[5];

	// zero incremental displacements
	m_du[0] = m_dul[0] = 0.0;
	m_du[1] = m_dul[1] = 0.0;
	m_du[2] = m_dul[2] = 0.0;
	m_du[3] = m_dul[3] = 0.0;
	m_du[4] = m_dul[4] = 0.0;
	m_du[5] = m_dul[5] = 0.0;
}

//-----------------------------------------------------------------------------
//! Set the rigid body's center of mass directly
void FERigidBody::SetCOM(vec3d rc)
{
	m_r0 = m_rt = rc;
}

//-----------------------------------------------------------------------------
//! Calculates the rigid body's total mass, center of mass, and mass moment
//! of inertia about the center of mass
//!
void FERigidBody::UpdateCOM()
{
	// get the mesh
	FEMesh& mesh = m_fem.GetMesh();

	// initialize some data
	m_mass = 0;			// total mass of rigid body
	vec3d rc(0,0,0);	// center of mass
    mat3d moi(0,0,0,0,0,0,0,0,0);    // mass moment of inertia about origin
    mat3dd I(1);        // identity tensor

	// jacobian
	double detJ;

	// shape function values
	double* H;

	// nodal coordinates
	vec3d r0[FEElement::MAX_NODES];
	
	// loop over all elements
	for (int nd=0; nd < mesh.Domains(); ++nd)
	{
		// TODO: I should convert to a FERigidSolidDomain or FERigidShellDomain
		if (mesh.Domain(nd).Class() == FE_DOMAIN_SOLID)
		{
			FESolidDomain* pbd = static_cast<FESolidDomain*>(&mesh.Domain(nd));
			FERigidMaterial* pm = dynamic_cast<FERigidMaterial*>(pbd->GetMaterial());
			// make sure this element belongs to the rigid body
			if (pm && (pm->GetRigidBodyID() == m_nID))
			{
				// get the material density
				FEParamDouble& density = pm->Density();

				// loop over all elements
				for (int iel=0; iel<pbd->Elements(); ++iel)
				{	
					FESolidElement& el = pbd->Element(iel);

					// nr of integration points
					int nint = el.GaussPoints();

					// number of nodes
					int neln = el.Nodes();

					// initial coordinates
					for (int i=0; i<neln; ++i) r0[i] = pbd->GetMesh()->Node(el.m_node[i]).m_r0;

					// integration weights
					double* gw = el.GaussWeights();

					// loop over integration points
					for (int n=0; n<nint; ++n)
					{
						FEMaterialPoint& mp = *el.GetMaterialPoint(n);

						// calculate jacobian
						detJ = pbd->detJ0(el, n);

						// shape functions at integration point
						H = el.H(n);

						double dens = density(mp);

						// add to total mass
						m_mass += dens*detJ*gw[n];

						// add to com and moi
						for (int i=0; i<el.Nodes(); ++i)
						{
							rc += r0[i]*H[i]*detJ*gw[n]*dens;
                            for (int j=0; j<el.Nodes(); ++j) {
                                moi += ((r0[i]*r0[j])*I - (r0[i] & r0[j]))*H[i]*H[j]*detJ*gw[n]*dens;
                            }
						}
					}
				}
			}
		}
	}

	// normalize com
	if (m_mass != 0) rc /= m_mass;
    
    // use parallel axis theorem to transfer moi to com
    // and store moi
    m_moi = moi.sym() - m_mass*((rc*rc)*I - dyad(rc));

	// store com
	m_r0 = m_rt = rc;
}

//-----------------------------------------------------------------------------
vec3d FERigidBody::CayleyIncrementalCompoundRotation()
{
    // incremental rotation in spatial frame
    quatd q = m_qt*m_qp.Inverse();
    q.MakeUnit();                           // clean-up roundoff errors
    double theta = 2*tan(q.GetAngle()/2);   // get theta from Cayley transform
    vec3d e = q.GetVector();
    
    return e*theta;
}

//-----------------------------------------------------------------------------
void FERigidBody::Serialize(DumpStream& ar)
{
	if (ar.IsShallow())
	{
		if (ar.IsSaving())
		{
			ar << m_mass;
			ar << m_moi;
			ar << m_Fr << m_Mr;
			ar << m_rp << m_rt;
			ar << m_vp << m_vt;
			ar << m_ap << m_at;
			ar << m_qp << m_qt << m_euler;
			ar << m_wp << m_wt;
			ar << m_alp << m_alt;
			for (int i=0; i<6; ++i)
			{
				ar << m_Up[i];
				ar << m_Ut[i];
				ar << m_du[i];
				ar << m_dul[i];
			}
		}
		else
		{
			ar >> m_mass;
			ar >> m_moi;
			ar >> m_Fr >> m_Mr;
			ar >> m_rp >> m_rt;
			ar >> m_vp >> m_vt;
			ar >> m_ap >> m_at;
			ar >> m_qp >> m_qt >> m_euler;
			ar >> m_wp >> m_wt;
			ar >> m_alp >> m_alt;
			for (int i=0; i<6; ++i)
			{
				ar >> m_Up[i];
				ar >> m_Ut[i];
				ar >> m_du[i];
				ar >> m_dul[i];
			}
		}
	}
	else
	{
		if (ar.IsSaving())
		{
			ar << m_nID << m_mat << m_mass << m_moi << m_Fr << m_Mr;
			ar << m_r0 << m_rt << m_rp << m_vt << m_vp << m_at << m_ap;
			ar << m_qt << m_qp << m_euler << m_wt << m_wp << m_alt << m_alp;
			ar << m_bpofr;
			ar.write(m_BC , sizeof(int), 6);
			ar.write(m_LM , sizeof(int), 6);
			ar.write(m_Up , sizeof(double), 6);
			ar.write(m_Ut , sizeof(double), 6);
			ar.write(m_du , sizeof(double), 6);
			ar.write(m_dul, sizeof(double), 6);
		}
		else
		{
			ar >> m_nID >> m_mat >> m_mass >> m_moi >> m_Fr >> m_Mr;
			ar >> m_r0 >> m_rt >> m_rp >> m_vt >> m_vp >> m_at >> m_ap;
			ar >> m_qt >> m_qp >> m_euler >> m_wt >> m_wp >> m_alt >> m_alp;
			ar >> m_bpofr;
			ar.read(m_BC , sizeof(int), 6);
			ar.read(m_LM , sizeof(int   ), 6);
			ar.read(m_Up , sizeof(double), 6);
			ar.read(m_Ut , sizeof(double), 6);
			ar.read(m_du , sizeof(double), 6);
			ar.read(m_dul, sizeof(double), 6);
		}
	}
}
