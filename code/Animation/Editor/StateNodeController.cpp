/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateNodeController.h"

#include "Animation/IPoseControllerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeController", 0, StateNodeController, StateNode)

StateNodeController::StateNodeController(const std::wstring& name, const IPoseControllerData* poseController)
	: StateNode(name)
	, m_poseController(poseController)
{
}

void StateNodeController::serialize(ISerializer& s)
{
	StateNode::serialize(s);
	s >> MemberRef(L"poseController", m_poseController);
}

}
