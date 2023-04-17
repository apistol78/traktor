/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraph.h"
#include "Render/Test/CaseRenderGraph.h"

namespace traktor::render::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.test.CaseRenderGraph", 0, CaseRenderGraph, traktor::test::Case)

void CaseRenderGraph::run()
{
	Ref< RenderGraph > rg = new RenderGraph(nullptr, 0);

	RenderGraphTargetSetDesc desc;
	desc.count = 1;
	desc.width = 64;
	desc.height = 64;
	desc.targets[0].colorFormat = TfR8G8B8A8;
	auto target = rg->addTransientTargetSet(L"Target", desc);

	{
		Ref< RenderPass > rp = new RenderPass();
		rp->addInput(target);
		rp->setOutput(target);
		rg->addPass(rp);
	}

	{
		Ref< RenderPass > rp = new RenderPass();
		rp->addInput(target);
		rp->setOutput(0);
		rg->addPass(rp);
	}

	bool result = rg->validate();
	CASE_ASSERT(result);
}

}
