/*This file is part of the FEBio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio.txt for details.

Copyright (c) 2019 University of Utah, Columbia University, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "FEErosionAdaptor.h"
#include <FECore/FEModel.h>
#include <FECore/FEMesh.h>
#include <FECore/FEDomain.h>
#include <FECore/log.h>
#include <FECore/FELinearConstraintManager.h>
#include <algorithm>

BEGIN_FECORE_CLASS(FEErosionAdaptor, FEMeshAdaptor)
	ADD_PARAMETER(m_maxIters, "max_iters");

	ADD_PROPERTY(m_criterion, "criterion");
END_FECORE_CLASS();

FEErosionAdaptor::FEErosionAdaptor(FEModel* fem) : FEMeshAdaptor(fem)
{
	m_maxIters = -1;
	m_criterion = nullptr;
}

bool FEErosionAdaptor::Apply(int iteration)
{
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	if ((m_maxIters >= 0) && (iteration >= m_maxIters))
	{
		feLog("\tMax iterations reached.");
		return true;
	}

	if (m_criterion == nullptr) return true;

	std::vector<int> selection = m_criterion->GetElementList();
	if (selection.empty())
	{
		feLog("\tNothing to do.\n");
		return true;
	}

	vector<int> elemList(mesh.Elements(), 0);
	for (int i = 0; i < selection.size(); ++i) elemList[selection[i]] = 1;

	int deactiveElems = 0;
	int elem = 0;
	for (int i = 0; i < mesh.Domains(); ++i)
	{
		FEDomain& dom = mesh.Domain(i);
		int NE = dom.Elements();
		for (int j = 0; j < NE; ++j, elem++)
		{
			if (elemList[elem] == 1)
			{
				FEElement& el = dom.ElementRef(j);
				assert(el.isActive());
				el.setInactive();
				deactiveElems++;
			}
		}
	}

	// if any nodes were orphaned, we need to deactivate them as well
	int NN = mesh.Nodes();
	vector<int> tag(NN, 0);
	for (int i = 0; i < mesh.Domains(); ++i)
	{
		FEDomain& dom = mesh.Domain(i);
		int NE = dom.Elements();
		for (int j = 0; j < NE; ++j)
		{
			FEElement& el = dom.ElementRef(j);
			if (el.isActive())
			{
				int neln = el.Nodes();
				for (int n = 0; n < neln; ++n) tag[el.m_node[n]] = 1;
			}
		}
	}

	for (int i = 0; i < NN; ++i)
	{
		FENode& node = mesh.Node(i);
		if (tag[i] == 0)
		{
			node.SetFlags(FENode::EXCLUDE);
			int ndofs = node.dofs();
			for (int j = 0; j < ndofs; ++j)
				node.set_inactive(j);
		}
	}

	// remove any linear constraints of exclude nodes
	FELinearConstraintManager& LCM = fem.GetLinearConstraintManager();
	for (int j = 0; j < LCM.LinearConstraints();)
	{
		FELinearConstraint& lc = LCM.LinearConstraint(j);
		if (mesh.Node(lc.master.node).HasFlags(FENode::EXCLUDE))
		{
			LCM.RemoveLinearConstraint(j);
		}
		else ++j;
	}

	// also remove any linear constraints that have slave excluded nodes
	for (int j = 0; j < LCM.LinearConstraints();)
	{
		FELinearConstraint& lc = LCM.LinearConstraint(j);

		bool del = false;
		int n = lc.slave.size();
		for (int k = 0; k < n; ++k)
		{
			if (mesh.Node(lc.slave[k].node).HasFlags(FENode::EXCLUDE))
			{
				del = true;
				break;
			}
		}
		if (del) LCM.RemoveLinearConstraint(j); else ++j;
	}

	// reactivate the linear constraints
	LCM.Activate();

	feLog("\tDeactivated elements: %d\n", deactiveElems);
	return (deactiveElems == 0);
}