//
//  FEFergusonShellDomain.hpp
//  FECore
//
//  Created by Gerard Ateshian on 1/29/16.
//  Copyright © 2016 febio.org. All rights reserved.
//

#pragma once
#include "FEDomain.h"

//-----------------------------------------------------------------------------
//! Abstract base class for shell elements
class FEFergusonShellDomain : public FEDomain
{
public:
    //! constructor
    FEFergusonShellDomain(FEMesh* pm) : FEDomain(FE_DOMAIN_FERGUSON, pm) {}
    
    //! create storage for elements
    void create(int nsize) { m_Elem.resize(nsize); }
    
    //! return nr of elements
    int Elements() { return m_Elem.size(); }
    
    //! element access
    FEFergusonShellElement& Element(int n) { return m_Elem[n]; }
    FEElement& ElementRef(int n) { return m_Elem[n]; }
    
    int GetElementType() { return m_Elem[0].Type(); }
    
    //! Initialize elements
    void InitElements();
    
    //! Reset element data
    void Reset();
    
    // inverse jacobian with respect to reference frame
    double invjac0(FEFergusonShellElement& el, double J[3][3], int n);
    
    // jacobian with respect to reference frame
    double detJ0(FEFergusonShellElement& el, int n);
    
    //! calculate jacobian in current frame
    double detJt(FEFergusonShellElement& el, int n);
    
    //! calculates covariant basis vectors at an integration point
    void CoBaseVectors(FEFergusonShellElement& el, int j, vec3d g[3]);
    
    //! calculates contravariant basis vectors at an integration point
    void ContraBaseVectors(FEFergusonShellElement& el, int j, vec3d g[3]);
    
    //! calculates parametric derivatives of covariant basis vectors at an integration point
    void CoBaseVectorDerivatives(FEFergusonShellElement& el, int j, vec3d dg[3][3]);
    
    //! calculates parametric derivatives of contravariant basis vectors at an integration point
    void ContraBaseVectorDerivatives(FEFergusonShellElement& el, int j, vec3d dg[3][3]);
    
    // calculate deformation gradient
    double defgrad(FEFergusonShellElement& el, mat3d& F, int n);
    
    // inverse jacobian with respect to current frame
    double invjact(FEFergusonShellElement& el, double J[3][3], int n);
    
    //! Serialize domain data to archive
    void Serialize(DumpStream& ar);
    
protected:
    vector<FEFergusonShellElement>	m_Elem;	//!< array of elements
    int					m_dofU;
    int					m_dofV;
    int					m_dofW;
};
