#pragma once
#include "fecore_api.h"
#include "DOFS.h"
#include "vec3d.h"
#include <vector>

//-----------------------------------------------------------------------------
//! This class defines a finite element node

//! It stores nodal positions and nodal equations numbers and more.
//!
//! The m_ID array will store the equation number for the corresponding
//! degree of freedom. Its values can be (a) non-negative (0 or higher) which
//! gives the equation number in the linear system of equations, (b) -1 if the
//! dof is fixed, and (c) < -1 if the dof corresponds to a prescribed dof. In
//! that case the corresponding equation number is given by -ID-2.

class FECORE_API FENode
{
public:
	// Node status flags
	enum Status {
		EXCLUDE = 1,	// exclude node from analysis
		SHELL = 2,	// this node belongs to a shell
		RIGID_CLAMP = 4,	// this node should be clamped to a rigid body (only applies to shell nodes)
	};

public:
	//! default constructor
	FENode();

	//! copy constructor
	FENode(const FENode& n);

	//! assignment operator
	FENode& operator = (const FENode& n);

	//! Set the number of DOFS
	void SetDOFS(int n);

	//! Get the nodal ID
	int GetID() const { return m_nID; }

	//! Set the node ID
	void SetID(int n) { m_nID = n; }

	//! see if status flags are set
	bool HasFlags(unsigned int flags) const { return ((m_nstate & flags) != 0); }

	//! set the status flags
	void SetFlags(unsigned int flags) { m_nstate = flags; }

	//! get the status falgs
	unsigned int Flags() const { return m_nstate; }

protected:
	int		m_nID;	//!< nodal ID

public: // geometry data
	vec3d	m_r0;	//!< initial position
	vec3d	m_rt;	//!< current position

	vec3d	m_at;	//!< nodal acceleration

	vec3d	m_rp;	//!< position of node at previous time step
	vec3d	m_vp;	//!< previous velocity
	vec3d	m_ap;	//!< previous acceleration

	vec3d	m_Fr;	//!< nodal reaction forces

	vec3d   m_d0;   //!< initial director

public:	// rigid body data
	unsigned int	m_nstate;	//!< node state flags
	int				m_rid;		//!< rigid body number

public:
	double get(int n) const { return m_val[n]; }
	void set(int n, double v) { m_val[n] = v; }
	void inc(int n, double v) { m_val[n] += v; }
	void dec(int n, double v) { m_val[n] -= v; }
	vec3d get_vec3d(int i, int j, int k) const { return vec3d(m_val[i], m_val[j], m_val[k]); }
	void set_vec3d(int i, int j, int k, const vec3d& v) { m_val[i] = v.x; m_val[j] = v.y; m_val[k] = v.z; }

public:
	std::vector<int>		m_BC;	//!< boundary condition array
	std::vector<int>		m_ID;	//!< nodal equation numbers
	std::vector<double>		m_val;	//!< nodal DOF values
};