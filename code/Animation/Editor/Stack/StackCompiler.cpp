/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/Stack/StackCompiler.h"

#include "Animation/Editor/Stack/Stack.h"
#include "Animation/Editor/Stack/StackLayer.h"
#include "Animation/Stack/RtStackData.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StackCompiler", StackCompiler, Object)

Ref< RtStackData > StackCompiler::compile(const Stack* stack) const
{
	Ref< RtStackData > rtsd = new RtStackData();

	for (auto layer : stack->getLayers())
		rtsd->m_layers.push_back({ .name = layer->getName(),
			.animation = layer->getAnimation(),
			.poseController = layer->getPoseController() });

	return rtsd;
}

}
