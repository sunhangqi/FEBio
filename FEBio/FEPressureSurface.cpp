#include "StdAfx.h"
#include "FEPressureSurface.h"

//-----------------------------------------------------------------------------
//! calculates the stiffness contribution due to hydrostatic pressure

void FEPressureSurface::PressureStiffness(FESurfaceElement& el, matrix& ke)
{
	int i, j, n;

	int nint = el.GaussPoints();
	int neln = el.Nodes();

	// pressure at nodes
	double *pn = el.pt();

	// pressure at integration point
	double p;

	// gauss weights
	double* w = el.GaussWeights();

	// nodal coordinates
	vec3d* rt = el.rt();

	// jacobian
	double J[3][2];

	double t1, t2;
	double kab[3];

	ke.zero();

	double* N, *Gr, *Gs;

	// repeat over integration points
	for (n=0; n<nint; ++n)
	{
		N = el.H(n);
		Gr = el.Gr(n);
		Gs = el.Gs(n);

		// calculate pressure at integration point
		p = 0;
		for (i=0; i<neln; ++i) p += N[i]*pn[i];

		// calculate jacobian
		J[0][0] = J[0][1] = 0;
		J[1][0] = J[1][1] = 0;
		J[2][0] = J[2][1] = 0;
		for (i=0; i<neln; ++i)
		{
			J[0][0] += Gr[i]*rt[i].x; J[0][1] += Gs[i]*rt[i].x;
			J[1][0] += Gr[i]*rt[i].y; J[1][1] += Gs[i]*rt[i].y;
			J[2][0] += Gr[i]*rt[i].z; J[2][1] += Gs[i]*rt[i].z;
		}

		// calculate stiffness component
		for (i=0; i<neln; ++i)
			for (j=0; j<neln; ++j)
			{
				t1 = 0.5*(Gs[i]*N[j] - Gs[j]*N[i]);
				t2 = 0.5*(Gr[i]*N[j] - Gr[j]*N[i]);

				kab[0] = p*(J[0][0]*t1 - J[0][1]*t2)*w[n];
				kab[1] = p*(J[1][0]*t1 - J[1][1]*t2)*w[n];
				kab[2] = p*(J[2][0]*t1 - J[2][1]*t2)*w[n];

				ke[3*i  ][3*j  ] +=       0; //(0,0,0)*kab[0]+(0,0,1)*kab[1]+(0,0,2)*kab[2];
				ke[3*i  ][3*j+1] +=  kab[2]; //(0,1,0)*kab[0]+(0,1,1)*kab[1]+(0,1,2)*kab[2];
				ke[3*i  ][3*j+2] += -kab[1]; //(0,2,0)*kab[0]+(0,2,1)*kab[1]+(0,2,2)*kab[2];

				ke[3*i+1][3*j  ] += -kab[2]; //(1,0,0)*kab[0]+(1,0,1)*kab[1]+(1,0,2)*kab[2];
				ke[3*i+1][3*j+1] +=       0; //(1,1,0)*kab[0]+(1,1,1)*kab[1]+(1,1,2)*kab[2];
				ke[3*i+1][3*j+2] +=  kab[0]; //(1,2,0)*kab[0]+(1,2,1)*kab[1]+(1,2,2)*kab[2];

				ke[3*i+2][3*j  ] +=  kab[1]; //(2,0,0)*kab[0]+(2,0,1)*kab[1]+(2,0,2)*kab[2];
				ke[3*i+2][3*j+1] += -kab[0]; //(2,1,0)*kab[0]+(2,1,1)*kab[1]+(2,1,2)*kab[2];
				ke[3*i+2][3*j+2] +=       0; //(2,2,0)*kab[0]+(2,2,1)*kab[1]+(2,2,2)*kab[2];
			}
	}
}

//-----------------------------------------------------------------------------
//! calculates the equivalent nodal forces due to hydrostatic pressure

bool FEPressureSurface::PressureForce(FESurfaceElement& el, vector<double>& fe)
{
	int i, n;

	// nr integration points
	int nint = el.GaussPoints();

	// nr of element nodes
	int neln = el.Nodes();

	// pressure at nodes
	double *pn = el.pt();

	// nodal coordinates
	vec3d *rt = el.rt();

	double* Gr, *Gs;
	double* N;
	double* w  = el.GaussWeights();

	// pressure at integration points
	double pr;

	vec3d dxr, dxs;

	// force vector
	vec3d f;

	// repeat over integration points
	fe.zero();
	for (n=0; n<nint; ++n)
	{
		N  = el.H(n);
		Gr = el.Gr(n);
		Gs = el.Gs(n);

		pr = 0;
		dxr = dxs = vec3d(0,0,0);
		for (i=0; i<neln; ++i) 
		{
			pr += N[i]*pn[i];

			dxr.x += Gr[i]*rt[i].x;
			dxr.y += Gr[i]*rt[i].y;
			dxr.z += Gr[i]*rt[i].z;

			dxs.x += Gs[i]*rt[i].x;
			dxs.y += Gs[i]*rt[i].y;
			dxs.z += Gs[i]*rt[i].z;
		}

		f = (dxr ^ dxs)*pr*w[n];

		for (i=0; i<neln; ++i)
		{
			fe[3*i  ] += N[i]*f.x;
			fe[3*i+1] += N[i]*f.y;
			fe[3*i+2] += N[i]*f.z;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
//! calculates the equivalent nodal forces due to hydrostatic pressure

bool FEPressureSurface::LinearPressureForce(FESurfaceElement& el, vector<double>& fe)
{
	int i, n;

	// nr integration points
	int nint = el.GaussPoints();

	// nr of element nodes
	int neln = el.Nodes();

	// pressure at nodes
	double *pn = el.pt();

	// nodal coordinates
	vec3d *r0 = el.r0();

	double* Gr, *Gs;
	double* N;
	double* w  = el.GaussWeights();

	// pressure at integration points
	double pr;

	vec3d dxr, dxs;

	// force vector
	vec3d f;

	// repeat over integration points
	fe.zero();
	for (n=0; n<nint; ++n)
	{
		N  = el.H(n);
		Gr = el.Gr(n);
		Gs = el.Gs(n);

		pr = 0;
		dxr = dxs = vec3d(0,0,0);
		for (i=0; i<neln; ++i) 
		{
			pr += N[i]*pn[i];

			dxr.x += Gr[i]*r0[i].x;
			dxr.y += Gr[i]*r0[i].y;
			dxr.z += Gr[i]*r0[i].z;

			dxs.x += Gs[i]*r0[i].x;
			dxs.y += Gs[i]*r0[i].y;
			dxs.z += Gs[i]*r0[i].z;
		}

		f = (dxr ^ dxs)*pr*w[n];

		for (i=0; i<neln; ++i)
		{
			fe[3*i  ] += N[i]*f.x;
			fe[3*i+1] += N[i]*f.y;
			fe[3*i+2] += N[i]*f.z;
		}
	}

	return true;
}
